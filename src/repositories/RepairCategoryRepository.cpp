#include "RepairCategoryRepository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

std::vector<RepairCategory> RepairCategoryRepository::getAll() {
    std::vector<RepairCategory> categories;
    QSqlQuery query("SELECT category_id, category_name FROM repair_categories");

    while (query.next()) {
        RepairCategory c;
        c.id = query.value(0).toInt();
        c.name = query.value(1).toString();
        categories.push_back(c);
    }
    return categories;
}

std::optional<RepairCategory> RepairCategoryRepository::getById(int id) {
    QSqlQuery query;
    query.prepare("SELECT category_id, category_name FROM repair_categories WHERE category_id = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        RepairCategory c;
        c.id = query.value(0).toInt();
        c.name = query.value(1).toString();
        return c;
    }
    return std::nullopt;
}

bool RepairCategoryRepository::insert(const RepairCategory& category) {
    QSqlQuery query;
    query.prepare("INSERT INTO repair_categories (category_name) VALUES (:name)");
    query.bindValue(":name", category.name);
    return query.exec();
}

bool RepairCategoryRepository::update(const RepairCategory& category) {
    QSqlQuery query;
    query.prepare("UPDATE repair_categories SET category_name = :name WHERE category_id = :id");
    query.bindValue(":name", category.name);
    query.bindValue(":id", category.id);
    return query.exec();
}

bool RepairCategoryRepository::deleteById(int id) {
    QSqlQuery query;
    query.prepare("DELETE FROM repair_categories WHERE category_id = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        qDebug() << "Помилка видалення категорії:" << query.lastError().text();
        return false;
    }
    return true;
}