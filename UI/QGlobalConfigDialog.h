#pragma once
#include <QDialog>
#include <QButtonGroup>
#include <QVBoxLayout>
class QGlobalConfigDialog :
    public QDialog
{
public:
    explicit QGlobalConfigDialog(QWidget* parent = nullptr);
private:
    QButtonGroup* mbgGroup=nullptr;
};

