#include "RepairServiceRepository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

std::vector<RepairService> RepairServiceRepository::getAll() {
    std::vector<RepairService> services;
    QSqlQuery query("SELECT service_id, category_id, service_name, description, base_price, estimated_days FROM repair_services");
    while (query.next()) {
        RepairService s;
        s.id = query.value(0).toInt();
        s.categoryId = query.value(1).toInt();
        s.name = query.value(2).toString();
        s.description = query.value(3).toString();
        s.basePrice = query.value(4).toDouble();
        s.estimatedDays = query.value(5).toInt();
        services.push_back(s);
    }
    return services;
}

std::optional<RepairService> RepairServiceRepository::getById(int id) {
    QSqlQuery query;
    query.prepare("SELECT service_id, category_id, service_name, description, base_price, estimated_days FROM repair_services WHERE service_id = :id");
    query.bindValue(":id", id);
    if (query.exec() && query.next()) {
        RepairService s;
        s.id = query.value(0).toInt();
        s.categoryId = query.value(1).toInt();
        s.name = query.value(2).toString();
        s.description = query.value(3).toString();
        s.basePrice = query.value(4).toDouble();
        s.estimatedDays = query.value(5).toInt();
        return s;
    }
    return std::nullopt;
}

bool RepairServiceRepository::insert(const RepairService& service) {
    QSqlQuery query;
    query.prepare("INSERT INTO repair_services (category_id, service_name, description, base_price, estimated_days) "
                  "VALUES (:catId, :name, :desc, :price, :days)");
    query.bindValue(":catId", service.categoryId);
    query.bindValue(":name", service.name);
    query.bindValue(":desc", service.description);
    query.bindValue(":price", service.basePrice);
    query.bindValue(":days", service.estimatedDays);
    return query.exec();
}

bool RepairServiceRepository::update(const RepairService& service) {
    QSqlQuery query;
    query.prepare("UPDATE repair_services SET category_id = :catId, service_name = :name, "
                  "description = :desc, base_price = :price, estimated_days = :days WHERE service_id = :id");
    query.bindValue(":catId", service.categoryId);
    query.bindValue(":name", service.name);
    query.bindValue(":desc", service.description);
    query.bindValue(":price", service.basePrice);
    query.bindValue(":days", service.estimatedDays);
    query.bindValue(":id", service.id);
    return query.exec();
}

bool RepairServiceRepository::deleteById(int id) {
    QSqlQuery query;
    query.prepare("DELETE FROM repair_services WHERE service_id = :id");
    query.bindValue(":id", id);
    return query.exec();
}