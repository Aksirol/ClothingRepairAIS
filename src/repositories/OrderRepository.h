#ifndef ORDERREPOSITORY_H
#define ORDERREPOSITORY_H

#include <vector>
#include <optional>
#include "Order.h"

class OrderRepository {
public:
    std::vector<Order> getAll();
    std::optional<Order> getById(int id);
    bool insert(const Order& order);
    bool update(const Order& order);
    bool deleteById(int id);
};

#endif