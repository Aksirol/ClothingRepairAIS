#ifndef ORDERITEMREPOSITORY_H
#define ORDERITEMREPOSITORY_H

#include <vector>
#include <optional>
#include "OrderItem.h"

class OrderItemRepository {
public:
    std::vector<OrderItem> getAll(); // Технічно потрібен рідко, частіше потрібен getAllByOrderId
    std::vector<OrderItem> getAllByOrderId(int orderId); // Додано для зручності виведення позицій конкретного замовлення
    std::optional<OrderItem> getById(int id);
    
    // Передаємо не константу, бо метод оновить поле unitPrice об'єкта після розрахунку
    bool insert(OrderItem& item); 
    
    bool update(const OrderItem& item);
    bool deleteById(int id);
};

#endif