#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_FactoryAutoTest.h"
#include "ui_GlobalStr.h"
#include "AutoTestView.h"
class FactoryAutoTestMain : public QMainWindow
{
    Q_OBJECT

public:
    FactoryAutoTestMain(QWidget *parent = Q_NULLPTR);
    bool eventFilter(QObject* watched, QEvent* event) override;
    void closeEvent(QCloseEvent*) override;
private slots:

    void on_action_about_triggered();
    void on_actionDevice_Driver_triggered();
    void on_action_settings_triggered();
    void on_action_GlobalConfig_triggered();
private:
    void setuprootactions();
    void WindowGeometry();
    Ui::FactoryAutoTestClass ui;
    AutoTestView* mainview=nullptr;
    QWidget* main_widget = nullptr;
};
