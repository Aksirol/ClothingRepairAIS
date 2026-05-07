#ifndef ORDERSTATUS_H
#define ORDERSTATUS_H

#include <QString>

struct OrderStatus {
    int id = -1;
    QString name;
    QString colorCode;
    bool isSystem = false; // Додано
};

#endif