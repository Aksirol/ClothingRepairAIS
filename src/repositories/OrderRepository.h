#ifndef ORDERREPOSITORY_H
#define ORDERREPOSITORY_H

#include <vector>
#include <optional>
#include "Order.h"

class OrderRepository {
public:
    std::vector<Order> getAll();
    std::vector<Order> getByClientId(int clientId); // ДОДАНО
    std::optional<Order> getById(int id);

    int insert(const Order& order); // ЗМІНЕНО: тепер повертає int (ID або -1)

    bool update(const Order& order);
    bool deleteById(int id);
};

#endif