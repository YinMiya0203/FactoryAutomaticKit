#ifndef COMBOBOX_DELEGATE_H
#define COMBOBOX_DELEGATE_H

#include <QString>
#include <QStyledItemDelegate>

#define ENABLE_SIZEHINT
class  TextEditDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    TextEditDelegate(QObject *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
                     const QModelIndex &index) const Q_DECL_OVERRIDE;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
        const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;

    void setModelData(QWidget *editor, QAbstractItemModel *model,
        const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const;
#ifdef ENABLE_SIZEHINT
    QSize sizeHint(const QStyleOptionViewItem& option,
        const QModelIndex& index) const Q_DECL_OVERRIDE;
#endif
};


#endif
