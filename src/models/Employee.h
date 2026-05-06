#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include <QString>
#include <QDate>

struct Employee {
    int id = -1;
    int positionId = -1; // Зовнішній ключ
    QString lastName;
    QString firstName;
    QString patronymic;
    QString phone;
    QDate hireDate;
};

#endif