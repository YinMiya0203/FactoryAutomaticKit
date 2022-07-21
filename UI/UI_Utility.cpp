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
	image.fill(bcolor);//È«Í¸Ã÷
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