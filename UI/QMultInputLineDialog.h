#pragma once
#include <QDialog>
#include <QLabel>
#include <QTimer>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLineEdit>
using InputWidgetContain = std::vector<std::shared_ptr<QLineEdit>>;
using InputResultContain = QStringList;
class QMultInputLineDialog :
    public QDialog
{
public:
    QMultInputLineDialog(QStringList prex,const QString &title=QString(), const QString& comment = QString(), QWidget* parent = nullptr);
    int Run(void);
    InputResultContain GetResult();
    ~QMultInputLineDialog();
private:
    InputWidgetContain minputwidgetcontain = {};
    InputResultContain minputresult = {};
};

