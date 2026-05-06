#ifndef ORDER_H
#define ORDER_H

#include <QString>
#include <QDate>

struct Order {
    int id = -1;
    int clientId = -1;
    int employeeId = -1;
    int statusId = -1;
    QDate receivedDate;
    QDate requiredDate;
    QDate completedDate; // Може бути недійсною (invalid), якщо замовлення ще не виконано
    double depositAmount = 0.0;
    QString paymentStatus = "Неоплачено"; // Додано
    QString notes;
};

#endif