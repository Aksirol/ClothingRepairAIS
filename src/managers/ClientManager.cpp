#include "ClientManager.h"
#include <QSqlQuery>
#include <QVariant>
#include <QDebug>

ClientManager::ClientManager() {}

bool ClientManager::registerClient(const Client& client) {
    // Перевірка на дублікати за телефоном
    QSqlQuery query;
    query.prepare("SELECT client_id FROM clients WHERE phone = :phone");
    query.bindValue(":phone", client.phone);
    
    if (query.exec() && query.next()) {
        qWarning() << "Клієнт із таким номером телефону вже існує!";
        return false; // Відмова у реєстрації
    }

    return clientRepo.insert(client);
}

std::vector<Client> ClientManager::searchClients(const QString& searchQuery) {
    std::vector<Client> results;
    QSqlQuery query;
    // Шукаємо збіги в прізвищі, імені або телефоні (LIKE %...%)
    query.prepare("SELECT client_id, last_name, first_name, patronymic, phone, email, address, registration_date "
                  "FROM clients WHERE last_name LIKE :search OR first_name LIKE :search OR phone LIKE :search");
    
    QString formattedSearch = "%" + searchQuery + "%";
    query.bindValue(":search", formattedSearch);

    if (query.exec()) {
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
            results.push_back(c);
        }
    }
    return results;
}

std::vector<Order> ClientManager::getClientHistory(int clientId) {
    // Тепер запит виконується на рівні БД! Швидко і ефективно.
    return orderRepo.getByClientId(clientId);
}