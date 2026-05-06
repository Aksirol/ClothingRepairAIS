#ifndef ORDERMANAGER_H
#define ORDERMANAGER_H

#include "OrderRepository.h"
#include "OrderItemRepository.h"
#include <vector>
#include <QString>

class OrderManager {
public:
    OrderManager();

    // Транзакційне створення замовлення разом із позиціями
    bool createOrderWithItems(const Order& order, std::vector<OrderItem>& items);

    // Зміна статусу з перевіркою бізнес-правил
    bool changeOrderStatus(int orderId, int newStatusId);

    // Оновлення статусу оплати
    bool updatePaymentStatus(int orderId, const QString& paymentStatus);

private:
    OrderRepository orderRepo;
    OrderItemRepository itemRepo;
};

#endif