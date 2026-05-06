#include "OrderStatusRepository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

std::vector<OrderStatus> OrderStatusRepository::getAll() {
    std::vector<OrderStatus> statuses;
    QSqlQuery query("SELECT status_id, status_name, color_code FROM order_statuses");

    while (query.next()) {
        OrderStatus s;
        s.id = query.value(0).toInt();
        s.name = query.value(1).toString();
        s.colorCode = query.value(2).toString();
        statuses.push_back(s);
    }
    return statuses;
}

std::optional<OrderStatus> OrderStatusRepository::getById(int id) {
    QSqlQuery query;
    query.prepare("SELECT status_id, status_name, color_code FROM order_statuses WHERE status_id = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        OrderStatus s;
        s.id = query.value(0).toInt();
        s.name = query.value(1).toString();
        s.colorCode = query.value(2).toString();
        return s;
    }
    return std::nullopt;
}

bool OrderStatusRepository::insert(const OrderStatus& status) {
    QSqlQuery query;
    query.prepare("INSERT INTO order_statuses (status_name, color_code) VALUES (:name, :color)");
    query.bindValue(":name", status.name);
    query.bindValue(":color", status.colorCode);
    return query.exec();
}

bool OrderStatusRepository::update(const OrderStatus& status) {
    QSqlQuery query;
    query.prepare("UPDATE order_statuses SET status_name = :name, color_code = :color WHERE status_id = :id");
    query.bindValue(":name", status.name);
    query.bindValue(":color", status.colorCode);
    query.bindValue(":id", status.id);
    return query.exec();
}

bool OrderStatusRepository::deleteById(int id) {
    QSqlQuery query;
    query.prepare("DELETE FROM order_statuses WHERE status_id = :id");
    query.bindValue(":id", id);
    return query.exec();
}