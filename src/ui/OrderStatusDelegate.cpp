#include "OrderStatusDelegate.h"
#include <QPainter>
#include <QColor>

OrderStatusDelegate::OrderStatusDelegate(int colorColumnIndex, QObject *parent)
    : QStyledItemDelegate(parent), colorColumn(colorColumnIndex) {}

void OrderStatusDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    // Отримуємо колір із прихованої колонки для поточного рядка
    QModelIndex colorIndex = index.siblingAtColumn(colorColumn);
    QString hexColor = colorIndex.data().toString();

    QStyleOptionViewItem opt = option;
    
    // Якщо є колір, робимо його напівпрозорим фоном
    if (!hexColor.isEmpty()) {
        QColor bgColor(hexColor);
        bgColor.setAlpha(60); // 60 із 255 — приємна напівпрозорість, щоб текст добре читався
        
        // Якщо рядок виділено користувачем, трохи змінюємо стиль
        if (opt.state & QStyle::State_Selected) {
            bgColor.setAlpha(120); 
        }
        
        painter->fillRect(opt.rect, bgColor);
    }

    // Малюємо стандартний текст поверх нашого фону
    QStyledItemDelegate::paint(painter, opt, index);
}