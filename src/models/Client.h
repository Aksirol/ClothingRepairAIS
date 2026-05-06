#ifndef CLIENT_H
#define CLIENT_H

#include <QString>
#include <QDate>

struct Client {
    int id = -1;
    QString lastName;
    QString firstName;
    QString patronymic;
    QString phone;
    QString email;
    QString address;
    QDate registrationDate;
};

#endif