#include "QCountDownDialog.h"
#include <QPainter>
#include <QFileDialog>
#include <QPainter>
#include <QVBoxLayout>
#include <QDesktopWidget>
#include <QImageReader>
#include "../TestCase.h"
#include "AudioEffect.h"
#include "UI_Utility.h"
QCountDownDialog::QCountDownDialog(QWidget* parent, int buttons, QList<QString> res_filler) :
    mCountStartvaluems(0), mcontinuevaluems(0)
{

    //setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    // 全透明
	//setAttribute(Qt::WA_TranslucentBackground);
    
	QVBoxLayout* pVBoxLayout = new QVBoxLayout(this);
    if(ShowResource(res_filler, pVBoxLayout)==0)//null invalid res
    {
	    m_timeLabel = new QLabel(this);
        pVBoxLayout->addWidget(m_timeLabel);

        m_timeLabel->setText("");
        m_timeLabel->setAlignment(Qt::AlignCenter);

        QString strStyle = "QLabel{"
            "font-family: \"Microsoft YaHei\";"
            "font-size: 128px;"
            "color: gray;"
            "}";
        m_timeLabel->setStyleSheet(strStyle);
    }
    this->setStyleSheet("QDialog{border:3px solid}");
    m_msgLabel = new QLabel(this);
    pVBoxLayout->addWidget(m_msgLabel);
    m_msgLabel->setText("");
    m_msgLabel->adjustSize();
    m_msgLabel->setAlignment(Qt::AlignCenter);
    cancelBtn = new QPushButton(this);
    pVBoxLayout->addWidget(cancelBtn);
    cancelBtn->setText(QStringLiteral("取消"));
    connect(cancelBtn, &QPushButton::clicked, this, &QCountDownDialog::on_cancelBtn_clicked);
    if (buttons > 1) {
        auto okBtn = new QPushButton(this);
        pVBoxLayout->addWidget(okBtn);
        okBtn->setText(QStringLiteral("确定"));
        connect(okBtn, &QPushButton::clicked, this, [this]() {
            m_pTimer->stop();
            this->accept();
            });
        okBtn->setFocus();
    }

    m_pTimer = new QTimer(this);
    connect(m_pTimer, &QTimer::timeout, this, &QCountDownDialog::on_timer_timeout);
    m_pTimer->setInterval(mcounterunit);
    AudioEffect::Warning();
    UI_Utility::SetFixtureBorderImage(this, QPalette::Window,30);
    UI_Utility::Planning_Layout(parent,this);
}
int32_t QCountDownDialog::ShowResource(QList<QString> res_filler, QVBoxLayout* pVBoxLayout)
{
    int ret = 0;
    //找出有多少res
    mfileInfo.clear();
    if (res_filler.size()==0) {
        goto ERROR_OUT;
    }
    {
        QString basedir = QFileInfo(TestcaseBase::get_instance()->get_runcasename()).absolutePath();
        QDir* dir = new QDir(basedir + "/Res/");
        QStringList filter;
        filter << res_filler;
        dir->setNameFilters(filter);
        mfileInfo = QList<QFileInfo>(dir->entryInfoList(filter));
        if (GlobalConfig_debugAutoTestView)qDebug("fileInfo size %d dir [%s]/[%s]", mfileInfo.size(), dir->absolutePath().toStdString().c_str(), res_filler.front().toStdString().c_str());
        if (mfileInfo.size() > 0) {
            auto res = mfileInfo.first().absoluteFilePath();
            auto m_resLabel = new QLabel(this);
            m_resLabel->setObjectName("m_resLabel");
            QRect screen = QDesktopWidget().screenGeometry();
            auto targetsize = screen.size() * 2 / 3;
            auto pixsize = QPixmap(res).size();
            auto targetscaledsize = pixsize.scaled(
                targetsize, Qt::KeepAspectRatio);
            if (GlobalConfig_debugAutoTestView)qDebug("targetscaledsize %d %d", targetscaledsize.width(), targetscaledsize.height());
            this->setFixedSize(targetscaledsize);
            pVBoxLayout->addWidget(m_resLabel);
            PlantResource();
        }
    }
ERROR_OUT:
    return mfileInfo.size();
}
void QCountDownDialog::PlantResource()
{

    if (mfileInfo.size() == 0)return;
    QString currentres = mfileInfo.at(mshowfileindex).absoluteFilePath();
    auto targetsize = this->size();

    auto pixmap = QPixmap(currentres).scaled(targetsize, Qt::KeepAspectRatio);
    auto m_resLabel = this->findChild<QLabel*>(QString("m_resLabel"));
    if(m_resLabel){
        m_resLabel->setPixmap(pixmap);
    }
    mshowfileindex++;
    mshowfileindex = (mshowfileindex % mfileInfo.size());
}
QCountDownDialog::~QCountDownDialog()
{
    qDebug("%p",this);
}
int QCountDownDialog::Run(int nvaluems, QString msg)
{
    m_pTimer->stop();
    mCountStartvaluems = nvaluems;
    m_msgLabel->setText(msg);
    if (nvaluems <=0) {
        nead_countdownaccept = false;
    }
    on_timer_timeout();
    m_pTimer->start();
    mcontinuevaluems = 0;
    return exec();
}
void QCountDownDialog::reject() {
    if (GlobalConfig_debugAutoTestView)qDebug("res size %d", mfileInfo.size());
    
    m_pTimer->stop();
    QDialog::reject();
}
void QCountDownDialog::on_cancelBtn_clicked()
{
    m_pTimer->stop();

    this->reject(); 

}
void QCountDownDialog::on_timer_timeout() 
{
    
    if (mcontinuevaluems >mCountStartvaluems && nead_countdownaccept) {
        m_pTimer->stop();
        if (GlobalConfig_debugAutoTestView)qDebug("res size %d", mfileInfo.size());
        
        if (mfileInfo.size() == 0)this->accept();
        else this->reject();
        return;
    }
    if (ui_time == 0 || (mcontinuevaluems- ui_time>1000)) {
        if(m_timeLabel)m_timeLabel->setText(QString("%1").arg(QString::number((mCountStartvaluems - mcontinuevaluems) /1000)));
        ui_time = mcontinuevaluems;
        PlantResource();
        if (cancelBtn) {
            if(nead_countdownaccept)cancelBtn->setText(QStringLiteral("取消(%1)").arg((mCountStartvaluems-mcontinuevaluems) / 1000));
        }
    }
    mcontinuevaluems += mcounterunit;
}


