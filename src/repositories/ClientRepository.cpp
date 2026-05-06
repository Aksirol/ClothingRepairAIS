#include "ClientRepository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

std::vector<Client> ClientRepository::getAll() {
    std::vector<Client> clients;
    QSqlQuery query("SELECT client_id, last_name, first_name, patronymic, phone, email, address, registration_date FROM clients");

    while (query.next()) {
        Client c;
        c.id = query.value(0).toInt();
        c.lastName = query.value(1).toString();
        c.firstName = query.value(2).toString();
        c.patronymic = query.value(3).toString();
        c.phone = query.value(4).toString();
        c.email = query.value(5).toString();
        c.address = query.value(6).toString();
        c.registrationDate = QDate::fromString(query.value(7).toString(), Qt::ISODate);
        clients.push_back(c);
    }
    return clients;
}

std::optional<Client> ClientRepository::getById(int id) {
    QSqlQuery query;
    query.prepare("SELECT client_id, last_name, first_name, patronymic, phone, email, address, registration_date FROM clients WHERE client_id = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        Client c;
        c.id = query.value(0).toInt();
        c.lastName = query.value(1).toString();
        c.firstName = query.value(2).toString();
        c.patronymic = query.value(3).toString();
        c.phone = query.value(4).toString();
        c.email = query.value(5).toString();
        c.address = query.value(6).toString();
        c.registrationDate = QDate::fromString(query.value(7).toString(), Qt::ISODate);
        return c;
    }
    return std::nullopt;
}

bool ClientRepository::insert(const Client& client) {
    QSqlQuery query;
    // registration_date заповнюється базою даних автоматично (date('now')), тому ми його тут не передаємо
    query.prepare("INSERT INTO clients (last_name, first_name, patronymic, phone, email, address) "
                  "VALUES (:lName, :fName, :patr, :phone, :email, :address)");
    query.bindValue(":lName", client.lastName);
    query.bindValue(":fName", client.firstName);
    query.bindValue(":patr", client.patronymic);
    query.bindValue(":phone", client.phone);
    query.bindValue(":email", client.email);
    query.bindValue(":address", client.address);
    
    if (!query.exec()) {
        qDebug() << "Помилка додавання клієнта:" << query.lastError().text();
        return false;
    }
    return true;
}

bool ClientRepository::update(const Client& client) {
    QSqlQuery query;
    query.prepare("UPDATE clients SET last_name = :lName, first_name = :fName, patronymic = :patr, "
                  "phone = :phone, email = :email, address = :address WHERE client_id = :id");
    query.bindValue(":lName", client.lastName);
    query.bindValue(":fName", client.firstName);
    query.bindValue(":patr", client.patronymic);
    query.bindValue(":phone", client.phone);
    query.bindValue(":email", client.email);
    query.bindValue(":address", client.address);
    query.bindValue(":id", client.id);
    
    return query.exec();
}

bool ClientRepository::deleteById(int id) {
    QSqlQuery query;
    query.prepare("DELETE FROM clients WHERE client_id = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        qDebug() << "Помилка видалення клієнта (можливо, є пов'язані замовлення):" << query.lastError().text();
        return false;
    }
    return true;
}