#include "FactoryAutoTest.h"
#include "GlobalSettings.h"
#include <QtWidgets/QApplication>
#include "LoginDlg.h"
#include <QSharedMemory>
#include <QMessageBox>
#include <QMutex>
#include <QFile>
#include <QDateTime>
#include <QTextStream>
#include <iostream>
#include <QDir>
#include <sstream>
static QString log_file="";
static QString GetLogfile(bool update=false) {
    if (log_file.size()==0|| update) {
        auto mtime = QDateTime::currentDateTime();
        log_file = QString("%1/%2/%3.%4").arg(GLOBALSETTINGSINSTANCE->GetLogDirLocation(), mtime.toString("yyyy-MM-dd"), mtime.toString("hh-mm-ss"), "log");
        Utility::NewDir(QFileInfo(log_file).absolutePath());
        qInfo("FactoryAutoTest %s\n", GetLogfile().toLocal8Bit().constData());
        //printf("FactoryAutoTest %s\n", GetLogfile().toLocal8Bit().constData());
    }
    return log_file;
}
void logMessageOutputQt5(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    static qint64 max = 10485760;//10 * 1024 * 1024;
    static QMutex mutex;
    mutex.lock();
    QString text;
    switch (type) {
    case QtDebugMsg:
        text = QString("Debug:");
        break;
    case QtWarningMsg:
        text = QString("Warning:");
        break;
    case QtCriticalMsg:
        text = QString("Critical:");
        break;
    case QtFatalMsg:
        text = QString("Fatal:");
        abort();
    case QtInfoMsg:
        text = QString("Info:");
        break;
    default:
        break;
    }
    QString sort_function;
    if(context.function){
        auto function_log = QString(context.function).split("(");
        sort_function = function_log.size()>0? function_log[0]:QString(context.function);
    }
    std::thread::id this_thread_id = std::this_thread::get_id();
    std::stringstream sin;
    sin << this_thread_id;
    //QString message = QString("[%1] %2 [%3] [%4] [%5] %6").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
    //    .arg(text).arg(context.file).arg(context.function).arg(context.line).arg(msg).arg("\n");
    QString message = QString("[%1] [%2(%4)] [%3] [%5] %6 %7").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
            .arg(text).arg(sort_function).arg(QString(sin.str().c_str())).arg(context.line).arg(msg).arg("\n");
    if (/*type != QtInfoMsg*/true) {
        if (type > QtDebugMsg) {
            if (QtInfoMsg == type) {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY |
                    FOREGROUND_GREEN);
            }else{
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY |
                FOREGROUND_RED);
            }
        }else{
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY |
                FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        }
        printf("%s", message.toLocal8Bit().constData()); 
        //goto ERR_OUT;
    }
    if(type >= QtSystemMsg) {
        QFile file(GetLogfile());
        if (file.size() > max) { //超过指定大小时截取文件
            QFile logfile(GetLogfile(true));

            file.open(QIODevice::ReadWrite);
            logfile.open(QIODevice::ReadWrite);
            qint64 nresult = 4194304; //4 * 1024 * 1024;
            file.seek(file.size() - nresult);
            char lbuffer[256];
            int count;
            while ((count = file.read(lbuffer, 256)) == 256) {
                logfile.write(lbuffer, count);
            }
            logfile.write(message.toLatin1(), message.count());
            file.close();
            file.remove();
            logfile.close();
        }
        else {
            QTextStream text_stream(&file);
            file.open(QIODevice::ReadWrite | QIODevice::Append);
            text_stream << message << endl;
            file.flush();
            file.close(); //频繁开关卡不卡啊
        }
    }
ERR_OUT:
    mutex.unlock();
}
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setStyle("windows");
    static QSharedMemory* shareMem = new QSharedMemory("yahha");
    if(!shareMem->create(1)) {
        //创建失败
        QMessageBox::critical(NULL, QString::fromLocal8Bit("错误"), QString::fromLocal8Bit("已有打开应用"), QMessageBox::Yes);
        qApp->quit();
        return -1;
    }

    qInstallMessageHandler(logMessageOutputQt5);

    LoginDlg dlg;

    if (dlg.exec() == LoginDlg::Accepted) 
    {
        FactoryAutoTestMain w; //注意顺序
        w.show();
        a.exec();
    }

    return 0;
}
