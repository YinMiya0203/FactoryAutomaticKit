#include <QDialog>
#include <QFileDialog>
#include<QDebug>
#include "FactoryAutoTest.h"
#include "TestCase.h"
#include <QMessageBox>
#include <QLabel>
#include <QGridLayout> 
#include "GlobalSettings.h"
#include "QDeviceDriverRunDialog.h"
#include "QGlobalSettingsDialog.h"
#include "QGlobalConfigDialog.h"
FactoryAutoTestMain::FactoryAutoTestMain(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    //setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Tool|Qt::WindowDoesNotAcceptFocus);
    //ui.action_about->setToolTip(TestcaseBase::get_instance()->get_runcasename());这个应该是没有tooltip
#if 0//def UI_DEBUG
    //ui.statusBar->showMessage(MESSAGE_YAHAH);
    QLabel* statuslabel = new QLabel;
    statuslabel->setMinimumSize(150, 20);
    statuslabel->setFrameShape(QFrame::WinPanel);
    statuslabel->setFrameShadow(QFrame::Sunken);
    ui.statusBar->addWidget(statuslabel);
    statuslabel->setText(MESSAGE_YAHAH);
#endif
    this->setWindowTitle(QString("FactoryAutomation %1").arg(GLOBALSETTINGSINSTANCE->GetFixtureTag()));
    setuprootactions();

    main_widget = new QWidget(this);
    main_widget->setObjectName("AutoTestView");
    //main_widget->setStyleSheet("AutoTestView{background-image:url(:/res/image/bg.jpg)}");
    mainview = new AutoTestView(main_widget);
    
    setCentralWidget(main_widget);

}
bool FactoryAutoTestMain::eventFilter(QObject* watched, QEvent* event)
{
    //qDebug("type %d", event->type());
    if (event->type()== QEvent::MouseButtonPress) {
        if(GlobalConfig_debugFactoryAutoTest)qDebug("type %d", event->type());
    }
    
    return QMainWindow::eventFilter(watched, event);
}

void FactoryAutoTestMain::closeEvent(QCloseEvent*)
{
    //qInfo(" ");
    delete mainview;
}

void FactoryAutoTestMain::on_action_about_triggered()
{
    //TBD 显示各个版本
    QString versions = AutoTestView::GetVerionInfo();
    QMessageBox::about(nullptr, VER_STR ,versions);
}

void FactoryAutoTestMain::on_actionDevice_Driver_triggered()
{
    auto dlg = new QDeviceDriverRunDialog;
    dlg->exec();
    delete dlg;
}

void FactoryAutoTestMain::on_action_settings_triggered()
{
    auto win = new QGlobalSettingsDialog();
    if (win->exec() != QDialog::Accepted) {
        GLOBALSETTINGSINSTANCE->LoadDefaultSettings();
    }
    delete win;
}

void FactoryAutoTestMain::on_action_GlobalConfig_triggered()
{
    auto dlg = new QGlobalConfigDialog;
    dlg->exec();
    delete dlg;
}

void FactoryAutoTestMain::setuprootactions()
{
    //QMenu* menu_root = new QMenu(menuBar);
    qInfo("GlobalSettings::is_root %d", GLOBALSETTINGSINSTANCE->isUserRoot());
    ui.menuroot->menuAction()->setVisible(GLOBALSETTINGSINSTANCE->isUserRoot());

}
