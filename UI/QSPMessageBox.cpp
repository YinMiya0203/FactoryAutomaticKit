#include "QSPMessageBox.h"
#include <QDialogButtonBox>
#include <QPushButton>
#include <QPainter>
#include <QMessageBox>
#include <GlobalSettings.h>
#include <UI/UI_Utility.h>

QSPMessageBox::QSPMessageBox(QWidget *parent)
	: QMessageBox(parent)
{}

QMessageBox::StandardButton QSPMessageBox::information PUBLIC_PARAM
{
    return QSPMessageBox::privateshow(QMessageBox::Icon::Information, parent, title, text, buttons, defaultButton);
}

QMessageBox::StandardButton QSPMessageBox::warning PUBLIC_PARAM
{
    return QSPMessageBox::privateshow(QMessageBox::Icon::Warning, parent, title, text, buttons, defaultButton);
}
QMessageBox::StandardButton QSPMessageBox::critical PUBLIC_PARAM
{
    return QSPMessageBox::privateshow(QMessageBox::Icon::Critical, parent, title, text, buttons, defaultButton);
}

QMessageBox::StandardButton QSPMessageBox::question PUBLIC_PARAM
{
    return QSPMessageBox::privateshow(QMessageBox::Icon::Question, parent, title, text, buttons, defaultButton);
}
QMessageBox::StandardButton QSPMessageBox::privateshow(QMessageBox::Icon icon, QWidget * parent, const QString & title, const QString & text, StandardButtons buttons, StandardButton defaultButton)
{
    QMessageBox msgBox(icon, title, text, QMessageBox::NoButton, parent);
#if 1
    QSize size(30,30);
    QImage image(size, QImage::Format_ARGB32);
    image.fill(qRgba(0, 0, 0, 0));//全透明
    QPainter painter(&image);
    
    //painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
    QPen pen = painter.pen();
    pen.setColor(qRgba(0, 0, 0, 255));
    QFont m_font;// ("Helvetica");
    m_font.setPixelSize(size.height());
    painter.setPen(pen);
    painter.setFont(m_font);
    painter.drawText(image.rect(), Qt::AlignCenter, GLOBALSETTINGSINSTANCE->GetFixtureTag());
    //msgBox.setWindowIcon(QIcon(":/QtRawview/Res/machine.ico")); 可行
    QPixmap pixmap;
    pixmap.convertFromImage(image);
    msgBox.setWindowIcon(QIcon(pixmap));
    QDialogButtonBox* buttonBox = msgBox.findChild<QDialogButtonBox*>();
    Q_ASSERT(buttonBox != 0);
    uint mask = QMessageBox::FirstButton;
    while (mask <= QMessageBox::LastButton) {
        uint sb = buttons & mask;
        mask <<= 1;
        if (!sb)
            continue;
       {
        QPushButton* button = msgBox.addButton((QMessageBox::StandardButton)sb);
        // Choose the first accept role as the default
        if (msgBox.defaultButton())
            continue;
        if ((defaultButton == QMessageBox::NoButton && buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole)
            || (defaultButton != QMessageBox::NoButton && sb == uint(defaultButton)))
            msgBox.setDefaultButton(button);
        }
       }
    {
        msgBox.setAutoFillBackground(true);
        QPalette pal = msgBox.palette();
        //pal.setBrush(QPalette::Base, Qt::yellow);
        //pal.setBrush(QPalette::Window, Qt::yellow); //obsolete
       // QImage image(weather_wind->size(), QImage::Format_ARGB32);
        //qDebug("w/h %d/%d",  msgBox.size().width(), msgBox.size().height());
        QSize size(100, 100);
        QImage image(size, QImage::Format_ARGB32);
        image.fill(qRgba(240, 240, 240, 255));
        QPainter painter(&image);

        //painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
        QPen pen = painter.pen();
        pen.setColor(qRgba(195, 195, 195, 100));
        QFont m_font("Helvetica");
        m_font.setPixelSize(size.height());
        painter.setPen(pen);
        painter.setFont(m_font);
        painter.drawText(image.rect(), Qt::AlignCenter, GLOBALSETTINGSINSTANCE->GetFixtureTag());
        //auto families = m_font.families();
        //qDebug("families %s", families.join(",").toStdString().c_str());
        //image.save("A.png");
        //pal.setBrush(QPalette::Base, QBrush(image.scaled(weather_wind->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
        pal.setBrush(QPalette::Window, QBrush(image));
        msgBox.setPalette(pal);
    }
    UI_Utility::Planning_Layout(parent, &msgBox);
    if (msgBox.exec() == -1)
        return QMessageBox::Cancel;
    return msgBox.standardButton(msgBox.clickedButton());
#endif
}

QSPMessageBox::~QSPMessageBox()
{}
