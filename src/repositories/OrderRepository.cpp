#include "OrderRepository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

std::vector<Order> OrderRepository::getAll() {
    std::vector<Order> orders;
    QSqlQuery query("SELECT order_id, client_id, employee_id, status_id, "
                    "received_date, required_date, completed_date, deposit_amount, payment_status, notes FROM orders");

    while (query.next()) {
        Order o;
        o.id = query.value(0).toInt();
        o.clientId = query.value(1).toInt();
        o.employeeId = query.value(2).toInt();
        o.statusId = query.value(3).toInt();
        o.receivedDate = QDate::fromString(query.value(4).toString(), Qt::ISODate);
        o.requiredDate = QDate::fromString(query.value(5).toString(), Qt::ISODate);
        
        // Якщо дати завершення немає (NULL), QDate буде invalid, що є правильною поведінкою
        if (!query.value(6).isNull()) {
            o.completedDate = QDate::fromString(query.value(6).toString(), Qt::ISODate);
        }
        
        o.depositAmount = query.value(7).toDouble();
        o.paymentStatus  = query.value(8).toString();  // ← додати
        o.notes          = query.value(9).toString();
        orders.push_back(o);
    }
    return orders;
}

std::optional<Order> OrderRepository::getById(int id) {
    QSqlQuery query;
    query.prepare("SELECT order_id, client_id, employee_id, status_id, "
                  "received_date, required_date, completed_date, deposit_amount, payment_status, notes "
                  "FROM orders WHERE order_id = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        Order o;
        o.id = query.value(0).toInt();
        o.clientId = query.value(1).toInt();
        o.employeeId = query.value(2).toInt();
        o.statusId = query.value(3).toInt();
        o.receivedDate = QDate::fromString(query.value(4).toString(), Qt::ISODate);
        o.requiredDate = QDate::fromString(query.value(5).toString(), Qt::ISODate);
        if (!query.value(6).isNull()) {
            o.completedDate = QDate::fromString(query.value(6).toString(), Qt::ISODate);
        }
        o.depositAmount = query.value(7).toDouble();
        o.paymentStatus  = query.value(8).toString();  // ← додати
        o.notes          = query.value(9).toString();
        return o;
    }
    return std::nullopt;
}

std::vector<Order> OrderRepository::getByClientId(int clientId) {
    std::vector<Order> orders;
    QSqlQuery query;
    query.prepare("SELECT order_id, client_id, employee_id, status_id, "
                  "received_date, required_date, completed_date, deposit_amount, payment_status, notes "
                  "FROM orders WHERE client_id = :cid");
    query.bindValue(":cid", clientId);

    if (query.exec()) {
        while (query.next()) {
            Order o;
            o.id = query.value(0).toInt();
            o.clientId = query.value(1).toInt();
            o.employeeId = query.value(2).toInt();
            o.statusId = query.value(3).toInt();
            o.receivedDate = QDate::fromString(query.value(4).toString(), Qt::ISODate);
            o.requiredDate = QDate::fromString(query.value(5).toString(), Qt::ISODate);
            if (!query.value(6).isNull()) {
                o.completedDate = QDate::fromString(query.value(6).toString(), Qt::ISODate);
            }
            o.depositAmount = query.value(7).toDouble();
            o.paymentStatus = query.value(8).toString();
            o.notes         = query.value(9).toString();
            orders.push_back(o);
        }
    }
    return orders;
}

int OrderRepository::insert(const Order& order) {
    QSqlQuery query;
    query.prepare("INSERT INTO orders (client_id, employee_id, status_id, received_date, required_date, completed_date, deposit_amount, payment_status, notes) "
                  "VALUES (:cId, :eId, :sId, :recDate, :reqDate, :compDate, :deposit, :payStatus, :notes)");
    query.bindValue(":cId", order.clientId);
    query.bindValue(":eId", order.employeeId);
    query.bindValue(":sId", order.statusId);
    query.bindValue(":recDate", order.receivedDate.toString(Qt::ISODate));
    query.bindValue(":reqDate", order.requiredDate.toString(Qt::ISODate));
    if (order.completedDate.isValid()) { query.bindValue(":compDate", order.completedDate.toString(Qt::ISODate)); }
    else { query.bindValue(":compDate", QVariant()); }
    query.bindValue(":deposit", order.depositAmount);
    query.bindValue(":payStatus", order.paymentStatus);
    query.bindValue(":notes", order.notes);

    if (!query.exec()) {
        qDebug() << "Помилка створення замовлення:" << query.lastError().text();
        return -1; // Повертаємо помилку
    }
    // Ідіоматичний Qt-спосіб отримати ID останнього вставленого запису
    return query.lastInsertId().toInt();
}

bool OrderRepository::update(const Order& order) {
    QSqlQuery query;
    query.prepare("UPDATE orders SET client_id = :cId, employee_id = :eId, status_id = :sId, "
                  "received_date = :recDate, required_date = :reqDate, completed_date = :compDate, "
                  "deposit_amount = :deposit, payment_status = :payStatus, notes = :notes WHERE order_id = :id");
    query.bindValue(":cId", order.clientId);
    query.bindValue(":eId", order.employeeId);
    query.bindValue(":sId", order.statusId);
    query.bindValue(":recDate", order.receivedDate.toString(Qt::ISODate));
    query.bindValue(":reqDate", order.requiredDate.toString(Qt::ISODate));
    
    if (order.completedDate.isValid()) {
        query.bindValue(":compDate", order.completedDate.toString(Qt::ISODate));
    } else {
        query.bindValue(":compDate", QVariant());
    }
    
    query.bindValue(":deposit", order.depositAmount);
    query.bindValue(":payStatus", order.paymentStatus);
    query.bindValue(":notes", order.notes);
    query.bindValue(":id", order.id);

    return query.exec();
}

bool OrderRepository::deleteById(int id) {
    QSqlQuery query;
    query.prepare("DELETE FROM orders WHERE order_id = :id");
    query.bindValue(":id", id);
    // Якщо видаляється замовлення, всі його order_items видаляться автоматично завдяки ON DELETE CASCADE у базі
    return query.exec();
}