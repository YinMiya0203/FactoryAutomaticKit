#pragma once
#include <QDialog>
#include <QLabel>
#include <QTimer>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFileInfo>
class QCountDownDialog :
    public QDialog
{
public:
    explicit QCountDownDialog(QWidget* parent = nullptr,int buttons = 1,QString res_filler = "");
    int Run(int32_t nvaluems,QString msg);
    ~QCountDownDialog();
    void reject();
public slots:
    void on_timer_timeout();
    void on_cancelBtn_clicked();
private:
	int32_t ShowResource(QString, QVBoxLayout*);
	void PlantResource();
    QLabel* m_timeLabel=nullptr;
    QLabel* m_msgLabel = nullptr;
    QTimer* m_pTimer;
    QPushButton* cancelBtn = nullptr;
    int32_t mStartvaluems;
    int32_t mcounterunit = 100;
    int32_t ui_time = 0;
    QList<QFileInfo> mfileInfo = {};
    int32_t mshowfileindex=0;
};

