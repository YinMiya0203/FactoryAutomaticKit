#pragma once
#include <QImage>
#include <QPainter>
#include <QPalette>
class UI_Utility
{
public:
	static void SetFixtureBorderImage(QWidget* widget, QPalette::ColorRole role=QPalette::Base, 
		int width = 200,
		int fcolor = qRgba(195, 195, 195, 100), int bcolor = qRgba(240, 240, 240, 255));
	static void Planning_Layout(QWidget* parent, QWidget* widget);

};

