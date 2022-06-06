#pragma once
//#include <qdialog.h>
#include "TestCase.h"
#include <QComboBox>
#include <QVBoxLayout>
#include <QDialog>
#include <QObject>
class QDeviceDriverRunDialog :
    public QDialog
{
    Q_OBJECT
public:
    QDeviceDriverRunDialog();
public slots:
    void  mcomboBox_currentlndexChanged(int index);
    void mcomboBox_currentlndexChangedStr(const QString&);
private:
    void HandleVisaDriverIoctrl(VisaDriverIoctrl cmd);
    void HandleVisaDriverIoctrlIoReadWrite(VisaDriverIoctrl cmd);
    void HandleVisaDriverIoctrlReadRaw(VisaDriverIoctrl cmd);
    void HandleVisaDriverIoctrlWithParam(VisaDriverIoctrl cmd);
    void CleanTempWidget();
    QList<QWidget*> tmpwdigetcontain;
    QVBoxLayout* ptopVBoxLayout;
};

