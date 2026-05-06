#include "ReportBuilder.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

QList<QVariantMap> ReportBuilder::getActiveOrders() {
    QList<QVariantMap> report;
    QSqlQuery query("SELECT o.order_id, c.last_name || ' ' || c.first_name AS client_name, "
                    "e.last_name AS master_name, s.status_name, o.required_date "
                    "FROM orders o "
                    "JOIN clients c ON o.client_id = c.client_id "
                    "JOIN employees e ON o.employee_id = e.employee_id "
                    "JOIN order_statuses s ON o.status_id = s.status_id "
                    "WHERE o.status_id IN (1, 2)"); // 1-Прийнято, 2-В роботі

    while (query.next()) {
        QVariantMap row;
        row["order_id"] = query.value(0);
        row["client_name"] = query.value(1);
        row["master_name"] = query.value(2);
        row["status_name"] = query.value(3);
        row["required_date"] = query.value(4);
        report.append(row);
    }
    return report;
}

QList<QVariantMap> ReportBuilder::getOverdueOrders() {
    QList<QVariantMap> report;
    QSqlQuery query;
    query.prepare("SELECT o.order_id, c.last_name, o.required_date, s.status_name "
                  "FROM orders o "
                  "JOIN clients c ON o.client_id = c.client_id "
                  "JOIN order_statuses s ON o.status_id = s.status_id "
                  "WHERE o.required_date < :today AND o.status_id NOT IN (4, 5)"); // Не 4(Видано) і не 5(Скасовано)
    query.bindValue(":today", QDate::currentDate().toString(Qt::ISODate));
    
    if (query.exec()) {
        while (query.next()) {
            QVariantMap row;
            row["order_id"] = query.value(0);
            row["client_name"] = query.value(1);
            row["required_date"] = query.value(2);
            row["status_name"] = query.value(3);
            report.append(row);
        }
    }
    return report;
}

double ReportBuilder::getRevenueForPeriod(const QDate& startDate, const QDate& endDate) {
    QSqlQuery query;
    // Виторг = сума всіх deposit_amount для успішно виданих замовлень у заданий період
    query.prepare("SELECT SUM(deposit_amount) FROM orders "
                  "WHERE status_id = 4 AND completed_date BETWEEN :start AND :end");
    query.bindValue(":start", startDate.toString(Qt::ISODate));
    query.bindValue(":end", endDate.toString(Qt::ISODate));

    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }
    return 0.0;
}

QList<QVariantMap> ReportBuilder::getMasterWorkload() {
    QList<QVariantMap> report;
    // Рахуємо кількість активних замовлень (статуси 1, 2, 3) для кожного працівника
    QSqlQuery query("SELECT e.last_name || ' ' || e.first_name AS master_name, COUNT(o.order_id) AS active_orders "
                    "FROM employees e "
                    "LEFT JOIN orders o ON e.employee_id = o.employee_id AND o.status_id IN (1, 2, 3) "
                    "GROUP BY e.employee_id "
                    "ORDER BY active_orders DESC");

    while (query.next()) {
        QVariantMap row;
        row["master_name"] = query.value(0);
        row["active_orders"] = query.value(1);
        report.append(row);
    }
    return report;
}