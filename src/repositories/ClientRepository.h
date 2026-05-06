#ifndef CLIENTREPOSITORY_H
#define CLIENTREPOSITORY_H

#include <vector>
#include <optional>
#include "Client.h"

class ClientRepository {
public:
    std::vector<Client> getAll();
    std::optional<Client> getById(int id);
    bool insert(const Client& client);
    bool update(const Client& client);
    bool deleteById(int id);
};

#endif