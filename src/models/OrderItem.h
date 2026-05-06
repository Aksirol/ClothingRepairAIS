#ifndef ORDERITEM_H
#define ORDERITEM_H

#include <QString>

struct OrderItem {
    int id = -1;
    int orderId = -1;
    int serviceId = -1;
    int quantity = 1;
    double unitPrice = 0.0; // Заповниться автоматично при збереженні
    QString clothingDescription;
    QString itemNotes;
};

#endif