#include "TextEditDelegate.h"

#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QStandardItemModel>
#include <QCompleter>
#include <QTreeView>
#include <QHeaderView>
#include <QTextEdit>
#include <QPainter>


TextEditDelegate::TextEditDelegate( QObject *parent)
{
}
#ifdef ENABLE_SIZEHINT
QSize TextEditDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{

    QString strContent = index.data().toString();

    auto mfont = index.model()->data(index, Qt::FontRole);
    QFont font = mfont.value<QFont>();
    QTextEdit edit;
    edit.setFont(font);
    
    edit.setText(strContent);
    QTextDocument* doc = edit.document();
    doc->adjustSize(); //这一步不能少
    int docsize = int(doc->size().rheight());
    int addsize = doc->lineCount() * font.pointSize();
    int height = (docsize+ addsize + font.pointSize() - 1) / font.pointSize() * font.pointSize();//加5是根据界面显示进行优化调整的
    int width = option.rect.width() == 0 ? 50 : option.rect.width();
    //qDebug("[ [%s] option.rect.width() %d height %d(%d %d %d %d)]", strContent.toStdString().c_str(),width, height,
     //   docsize, font.pointSize(),doc->lineCount(), addsize);
    //height = 80;
    return QSize(width, height);


}
#endif
void TextEditDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();
    auto cl =  index.model()->data(index, Qt::BackgroundRole);
        QPen pen;
        if (option.state & QStyle::State_Selected ) {
            if( option.state & QStyle::State_HasFocus )
            {
                painter->fillRect(option.rect, option.palette.highlight());
                pen.setColor( Qt::white );

            }
            else
            {
                painter->fillRect(option.rect, option.palette.base() );
                pen.setColor( Qt::black );
            }
        }
        else
        {
            painter->fillRect(option.rect, option.palette.base() );
            pen.setColor( Qt::black );
        }

        painter->setPen( pen );
        painter->fillRect(option.rect, cl.value<QBrush>());//
        
        painter->drawText(option.rect,Qt::TextWrapAnywhere,value);

        //QStyledItemDelegate::paint(painter, option, index);
}

QWidget *TextEditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    QTextEdit *editor = new QTextEdit(parent);
    //editor->setWordWrapMode(QTextOption::WrapMode::WordWrap);
    return editor;
}

void TextEditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();

    QTextEdit *ComboBox = static_cast<QTextEdit*>(editor);

    ComboBox->setPlainText( value );


}

void TextEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QTextEdit *ComboBox = static_cast<QTextEdit*>(editor);

    //spinBox->interpretText();
    QString value = ComboBox->toPlainText();

    model->setData(index, value, Qt::EditRole);
}

void TextEditDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

