#include "UI_Utility.h"
#include <QWidget>
#include "GlobalSettings.h"
void UI_Utility::SetFixtureBorderImage(QWidget* widget, QPalette::ColorRole role, int width, int fcolor, int bcolor)
{
	if (widget == nullptr) { 
		qCritical("widget null");
		return; 
	}
	widget->setAutoFillBackground(true);
	QPalette pal = widget->palette();
	QSize size(width, width);
	QImage image(size, QImage::Format_ARGB32);
	image.fill(bcolor);//全透明
	QPainter painter(&image);
	QPen pen = painter.pen();
	pen.setColor(fcolor);
	QFont m_font("Helvetica");
	m_font.setPixelSize(size.height());
	painter.setPen(pen);
	painter.setFont(m_font);
	painter.drawText(image.rect(), Qt::AlignCenter, GLOBALSETTINGSINSTANCE->GetFixtureTag());
	pal.setBrush(role, QBrush(image));
	widget->setPalette(pal);
}
QWidget* TopLevelParentWidget(QWidget* widget)
{
//	if (widget)qDebug() << widget->objectName();
	while (widget->parentWidget() != Q_NULLPTR) {
		widget = widget->parentWidget();
//		if (widget)qDebug() << widget->objectName();
	} 
	return widget;
}
void UI_Utility::Planning_Layout(QWidget* parent, QWidget* widget)
{

	//显示在parent的1/4大小
	if (parent==nullptr || widget == nullptr) {
		qCritical("param invaild");
		return;
	}
	//return;
	QRect prect = parent->frameGeometry();
	QRect crect = widget->frameGeometry();
	int plantwidht = min(parent->frameGeometry().width()/2, crect.width());
	int plantheight = min(parent->frameGeometry().height() / 2, crect.height());
	QSize targetsize(plantwidht, plantheight);
	auto targetscaledsize=crect.size().scaled(
		targetsize, Qt::KeepAspectRatio);
	QWidget* topWidget = TopLevelParentWidget(parent);
	if (topWidget == nullptr) {
		qCritical("topWidget null");
		return;
	
	}
	auto point = topWidget->frameGeometry();
	//qDebug() << "this loc:" << point.x() << point.y();
	widget->move(point.x() + 25, point.y() + 25);
	widget->setFixedSize(targetscaledsize.width(), targetscaledsize.height());
}
