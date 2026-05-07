#ifndef ORDERSTATUSDELEGATE_H
#define ORDERSTATUSDELEGATE_H

#include <QStyledItemDelegate>

class OrderStatusDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    // Передаємо індекс колонки, де лежить HEX-код кольору
    explicit OrderStatusDelegate(int colorColumnIndex, QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    int colorColumn;
};

#endif