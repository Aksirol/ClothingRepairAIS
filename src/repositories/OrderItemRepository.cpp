#include "OrderItemRepository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

std::vector<OrderItem> OrderItemRepository::getAll() {
    std::vector<OrderItem> items;
    QSqlQuery query("SELECT item_id, order_id, service_id, quantity, unit_price, clothing_description, item_notes FROM order_items");

    while (query.next()) {
        OrderItem i;
        i.id = query.value(0).toInt();
        i.orderId = query.value(1).toInt();
        i.serviceId = query.value(2).toInt();
        i.quantity = query.value(3).toInt();
        i.unitPrice = query.value(4).toDouble();
        i.clothingDescription = query.value(5).toString();
        i.itemNotes = query.value(6).toString();
        items.push_back(i);
    }
    return items;
}

std::vector<OrderItem> OrderItemRepository::getAllByOrderId(int orderId) {
    std::vector<OrderItem> items;
    QSqlQuery query;
    query.prepare("SELECT item_id, order_id, service_id, quantity, unit_price, clothing_description, item_notes "
                  "FROM order_items WHERE order_id = :oId");
    query.bindValue(":oId", orderId);
    
    if (query.exec()) {
        while (query.next()) {
            OrderItem i;
            i.id = query.value(0).toInt();
            i.orderId = query.value(1).toInt();
            i.serviceId = query.value(2).toInt();
            i.quantity = query.value(3).toInt();
            i.unitPrice = query.value(4).toDouble();
            i.clothingDescription = query.value(5).toString();
            i.itemNotes = query.value(6).toString();
            items.push_back(i);
        }
    }
    return items;
}

std::optional<OrderItem> OrderItemRepository::getById(int id) {
    QSqlQuery query;
    query.prepare("SELECT item_id, order_id, service_id, quantity, unit_price, clothing_description, item_notes "
                  "FROM order_items WHERE item_id = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        OrderItem i;
        i.id = query.value(0).toInt();
        i.orderId = query.value(1).toInt();
        i.serviceId = query.value(2).toInt();
        i.quantity = query.value(3).toInt();
        i.unitPrice = query.value(4).toDouble();
        i.clothingDescription = query.value(5).toString();
        i.itemNotes = query.value(6).toString();
        return i;
    }
    return std::nullopt;
}

bool OrderItemRepository::insert(OrderItem& item) {
    // 1. АВТОМАТИЧНИЙ РОЗРАХУНОК: Беремо актуальну ціну з довідника послуг
    QSqlQuery priceQuery;
    priceQuery.prepare("SELECT base_price FROM repair_services WHERE service_id = :sId");
    priceQuery.bindValue(":sId", item.serviceId);
    
    if (priceQuery.exec() && priceQuery.next()) {
        item.unitPrice = priceQuery.value(0).toDouble(); // Копіюємо історичну ціну в об'єкт
    } else {
        qCritical() << "Помилка: неможливо знайти послугу з ID" << item.serviceId;
        return false; 
    }

    // 2. Вставляємо позицію замовлення
    QSqlQuery query;
    query.prepare("INSERT INTO order_items (order_id, service_id, quantity, unit_price, clothing_description, item_notes) "
                  "VALUES (:oId, :sId, :qty, :price, :desc, :notes)");
    query.bindValue(":oId", item.orderId);
    query.bindValue(":sId", item.serviceId);
    query.bindValue(":qty", item.quantity);
    query.bindValue(":price", item.unitPrice); // Використовуємо скопійовану ціну
    query.bindValue(":desc", item.clothingDescription);
    query.bindValue(":notes", item.itemNotes);

    if (!query.exec()) {
        qDebug() << "Помилка додавання позиції замовлення:" << query.lastError().text();
        return false;
    }
    return true;
}

bool OrderItemRepository::update(const OrderItem& item) {
    // При оновленні ми вже не підтягуємо ціну автоматично, 
    // щоб адміністратор мав можливість відредагувати ціну вручну при потребі.
    QSqlQuery query;
    query.prepare("UPDATE order_items SET order_id = :oId, service_id = :sId, quantity = :qty, "
                  "unit_price = :price, clothing_description = :desc, item_notes = :notes "
                  "WHERE item_id = :id");
    query.bindValue(":oId", item.orderId);
    query.bindValue(":sId", item.serviceId);
    query.bindValue(":qty", item.quantity);
    query.bindValue(":price", item.unitPrice);
    query.bindValue(":desc", item.clothingDescription);
    query.bindValue(":notes", item.itemNotes);
    query.bindValue(":id", item.id);

    return query.exec();
}

bool OrderItemRepository::deleteById(int id) {
    QSqlQuery query;
    query.prepare("DELETE FROM order_items WHERE item_id = :id");
    query.bindValue(":id", id);
    return query.exec();
}