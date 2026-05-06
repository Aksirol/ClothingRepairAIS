#ifndef ORDERSTATUSREPOSITORY_H
#define ORDERSTATUSREPOSITORY_H

#include <vector>
#include <optional>
#include "OrderStatus.h"

class OrderStatusRepository {
public:
    std::vector<OrderStatus> getAll();
    std::optional<OrderStatus> getById(int id);
    bool insert(const OrderStatus& status);
    bool update(const OrderStatus& status);
    bool deleteById(int id);
};

#endif