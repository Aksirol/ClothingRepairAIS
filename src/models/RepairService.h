#ifndef REPAIRSERVICE_H
#define REPAIRSERVICE_H

#include <QString>

struct RepairService {
    int id = -1;
    int categoryId = -1;
    QString name;
    QString description;
    double basePrice = 0.0;
    int estimatedDays = 0;
};

#endif