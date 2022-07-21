#pragma once

#include <QMessageBox>
#define PUBLIC_PARAM_DEFINE (QWidget* parent, const QString& title,	\
const QString& text, StandardButtons buttons = Ok,	\
StandardButton defaultButton = NoButton)

#define PUBLIC_PARAM (QWidget* parent, const QString& title,	\
const QString& text, StandardButtons buttons,	\
StandardButton defaultButton)

class QSPMessageBox  : public QMessageBox
{
	Q_OBJECT

public:
	QSPMessageBox(QWidget *parent);
	static StandardButton information PUBLIC_PARAM_DEFINE;
	static StandardButton warning PUBLIC_PARAM_DEFINE;
	static StandardButton critical PUBLIC_PARAM_DEFINE;
	static StandardButton question PUBLIC_PARAM_DEFINE;
	~QSPMessageBox();
private:
	static StandardButton privateshow(QMessageBox::Icon icon, QWidget* parent, const QString& title,
		const QString& text, StandardButtons buttons = Ok,
		StandardButton defaultButton = NoButton);
};
