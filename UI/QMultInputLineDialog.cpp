#include "QMultInputLineDialog.h"
#include <QRegExpValidator>
#include <QRegExp>
#include "AudioEffect.h"
#include "UI_Utility.h"

QMultInputLineDialog::QMultInputLineDialog(QStringList prexlist, QWidget* parent, const QString& title, const QString& comment)
{
	setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
	// 全透明
	//setAttribute(Qt::WA_TranslucentBackground);


	QVBoxLayout* ptopVBoxLayout = new QVBoxLayout;
	QHBoxLayout* phitemsBoxLayout = new QHBoxLayout;
	auto widgetitems = new QWidget;
	if (title.size() > 0) {
		setWindowTitle(title);
	}
	else {
		setWindowTitle(QStringLiteral("输入提示框"));
	}
	if (comment.size() > 0) {
		auto comment_label = new QLabel;
		comment_label->setText(comment);
		comment_label->setFont(QFont("Microsoft YaHei UI", 10));
		ptopVBoxLayout->addWidget(comment_label);
	}
	int offset = 0;
	foreach(auto pre ,prexlist) {
		QVBoxLayout* pitemVBoxLayout = new QVBoxLayout;
		auto widgetitem = new QWidget;
		auto prex_label = new QLabel;
		prex_label->setText(pre);
		prex_label->setFont(QFont("Microsoft YaHei UI", 15, QFont::Bold));
		pitemVBoxLayout->addWidget(prex_label);
		auto input_edit = new QLineEdit;
		QRegExpValidator* pRegVld = new QRegExpValidator(this);
		pRegVld->setRegExp(QRegExp(QString("^-?\\d{1,}(\\.[0-9]+)?")));
		input_edit->setValidator(pRegVld);
		input_edit->setObjectName(QString("input_edit_%1").arg(offset));
		std::shared_ptr<QLineEdit> ptr(input_edit);
		minputwidgetcontain.push_back(ptr);
		offset++;
		pitemVBoxLayout->addWidget(input_edit);
		widgetitem->setLayout(pitemVBoxLayout);
		phitemsBoxLayout->addWidget(widgetitem);
	}
	widgetitems->setLayout(phitemsBoxLayout);
	ptopVBoxLayout->addWidget(widgetitems);

	auto okBtn = new QPushButton(this);
	ptopVBoxLayout->addWidget(okBtn);
	okBtn->setText(QStringLiteral("确定"));
	connect(okBtn, &QPushButton::clicked, this, [this]() {
		minputresult.clear();
		foreach(auto pre, minputwidgetcontain) {
			QString gettext = pre->text();
			//printf("gettext %s", gettext.toStdString().c_str());
			minputresult.push_back(gettext);
		}
		this->accept();
		});
	auto cancelBtn = new QPushButton(this);
	ptopVBoxLayout->addWidget(cancelBtn);
	cancelBtn->setText(QStringLiteral("取消"));
	connect(cancelBtn, &QPushButton::clicked, this, [this]() {
		this->reject();
		});
	setLayout(ptopVBoxLayout);
	AudioEffect::Warning();
	UI_Utility::SetFixtureBorderImage(this, QPalette::Window, 30);
	UI_Utility::Planning_Layout(parent, this);
}

int QMultInputLineDialog::Run(void)
{
	return exec();
}

InputResultContain QMultInputLineDialog::GetResult()
{
	return minputresult;
}

QMultInputLineDialog::~QMultInputLineDialog()
{
	minputwidgetcontain.clear();
	minputresult.clear();
}
