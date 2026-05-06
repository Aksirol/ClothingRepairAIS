#include "PositionRepository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

std::vector<Position> PositionRepository::getAll() {
    std::vector<Position> positions;
    QSqlQuery query("SELECT position_id, position_name, hourly_rate FROM positions");

    while (query.next()) {
        Position p;
        p.id = query.value(0).toInt();
        p.name = query.value(1).toString();
        p.hourlyRate = query.value(2).toDouble();
        positions.push_back(p);
    }
    return positions;
}

bool PositionRepository::insert(const Position& position) {
    QSqlQuery query;
    query.prepare("INSERT INTO positions (position_name, hourly_rate) VALUES (:name, :rate)");
    query.bindValue(":name", position.name);
    query.bindValue(":rate", position.hourlyRate);

    if (!query.exec()) {
        qDebug() << "Insert error:" << query.lastError().text();
        return false;
    }
    return true;
}

bool PositionRepository::deleteById(int id) {
    QSqlQuery query;
    query.prepare("DELETE FROM positions WHERE position_id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        // Тут спрацює наш захист FOREIGN KEY, якщо на цій посаді є працівники
        qDebug() << "Delete error:" << query.lastError().text();
        return false;
    }
    return true;
}

// Методи getById та update реалізуються за таким же принципом...