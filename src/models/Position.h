#ifndef POSITION_H
#define POSITION_H

#include <QString>

struct Position {
    int id = -1;
    QString name;
    double hourlyRate = 0.0;
};

#endif