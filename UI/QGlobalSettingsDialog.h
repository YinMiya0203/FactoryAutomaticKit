#pragma once
#include <QDialog>
#include <QVBoxLayout>
class QGlobalSettingsDialog :
    public QDialog
{
public:
    QGlobalSettingsDialog(bool islogin=false);
    void on_okpb_clicked(int);
    //QList<QWidget*> tmpwdigetcontain;
    int32_t DeleteView(QWidget* dev_widget);
    QVBoxLayout* ptopVBoxLayout;
    void setupsettingview();
    QWidget* ptopwidght;
    
private:
    void windowCenter();
};

