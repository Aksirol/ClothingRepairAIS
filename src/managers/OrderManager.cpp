#include "OrderManager.h"
#include "OrderStatusIds.h" // Підключаємо константи
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <QDebug>

OrderManager::OrderManager() {}

bool OrderManager::createOrderWithItems(const Order& order, std::vector<OrderItem>& items) {
    if (items.empty()) {
        qWarning() << "Помилка: Замовлення повинно містити хоча б одну позицію.";
        return false;
    }

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction()) { return false; }

    // Використовуємо наш новий метод, що повертає ID
    int newOrderId = orderRepo.insert(order);
    if (newOrderId == -1) {
        db.rollback();
        return false;
    }

    for (auto& item : items) {
        item.orderId = newOrderId;
        if (!itemRepo.insert(item)) {
            qCritical() << "Помилка при збереженні позиції. Відкат транзакції.";
            db.rollback();
            return false;
        }
    }
    return db.commit();
}

bool OrderManager::changeOrderStatus(int orderId, int newStatusId) {
    auto orderOpt = orderRepo.getById(orderId);
    if (!orderOpt) return false;

    Order order = *orderOpt;

    // Використовуємо константи
    if (order.statusId == StatusId::Issued || order.statusId == StatusId::Cancelled) {
        qWarning() << "Заборонено змінювати статус для виданих або скасованих замовлень.";
        return false;
    }

    order.statusId = newStatusId;

    if (newStatusId == StatusId::Ready) {
        order.completedDate = QDate::currentDate();
    }

    return orderRepo.update(order);
}

bool OrderManager::updatePaymentStatus(int orderId, const QString& paymentStatus) {
    auto orderOpt = orderRepo.getById(orderId);
    if (!orderOpt) return false;
    
    Order order = *orderOpt;
    order.paymentStatus = paymentStatus;
    
    return orderRepo.update(order);
}

bool OrderManager::updateOrderWithItems(const Order& order, std::vector<OrderItem>& items) {
    if (items.empty()) {
        qWarning() << "Помилка: Замовлення повинно містити хоча б одну позицію.";
        return false;
    }

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction()) { return false; }

    // 1. Оновлюємо "шапку" замовлення
    if (!orderRepo.update(order)) {
        db.rollback();
        return false;
    }

    // 2. Видаляємо старі позиції (найчистіший підхід до синхронізації колекцій)
    QSqlQuery delQuery;
    delQuery.prepare("DELETE FROM order_items WHERE order_id = :id");
    delQuery.bindValue(":id", order.id);
    if (!delQuery.exec()) {
        db.rollback();
        return false;
    }

    // 3. Вставляємо нові/відредаговані позиції
    for (auto& item : items) {
        item.orderId = order.id;
        item.id = -1; // Скидаємо ID, щоб база згенерувала нові
        if (!itemRepo.insert(item)) {
            db.rollback();
            return false;
        }
    }

    return db.commit();
}