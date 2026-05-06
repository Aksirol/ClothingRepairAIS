#ifndef CLIENTMANAGER_H
#define CLIENTMANAGER_H

#include "ClientRepository.h"
#include "OrderRepository.h"
#include <vector>
#include <QString>

class ClientManager {
public:
    ClientManager();

    // Реєстрація з перевіркою на дублікати
    bool registerClient(const Client& client);

    // Глобальний пошук (за ПІБ або телефоном)
    std::vector<Client> searchClients(const QString& searchQuery);

    // Історія замовлень клієнта
    std::vector<Order> getClientHistory(int clientId);

private:
    ClientRepository clientRepo;
    OrderRepository orderRepo;
};

#endif