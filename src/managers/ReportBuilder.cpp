#include "ReportBuilder.h"
#include "OrderStatusIds.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

QList<QVariantMap> ReportBuilder::getActiveOrders() {
    QList<QVariantMap> report;
    QSqlQuery query;
    query.prepare("SELECT o.order_id, c.last_name || ' ' || c.first_name AS client_name, "
                  "e.last_name AS master_name, s.status_name, o.required_date "
                  "FROM orders o "
                  "JOIN clients c ON o.client_id = c.client_id "
                  "JOIN employees e ON o.employee_id = e.employee_id "
                  "JOIN order_statuses s ON o.status_id = s.status_id "
                  "WHERE o.status_id IN (:s1, :s2)");
    query.bindValue(":s1", StatusId::Accepted);
    query.bindValue(":s2", StatusId::InProgress);

    if (query.exec()) {
        while (query.next()) {
            QVariantMap row;
            row["order_id"] = query.value(0);
            row["client_name"] = query.value(1);
            row["master_name"] = query.value(2);
            row["status_name"] = query.value(3);
            row["required_date"] = query.value(4);
            report.append(row);
        }
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
                  "WHERE o.required_date < :today AND o.status_id NOT IN (:s1, :s2)");
    query.bindValue(":today", QDate::currentDate().toString(Qt::ISODate));
    query.bindValue(":s1", StatusId::Issued);
    query.bindValue(":s2", StatusId::Cancelled);

    // Залишили лише один виклик exec()
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
    query.prepare("SELECT COALESCE(SUM(oi.quantity * oi.unit_price), 0.0) "
                  "FROM orders o "
                  "JOIN order_items oi ON o.order_id = oi.order_id "
                  "WHERE o.status_id = :statusId "
                  "AND o.completed_date BETWEEN :start AND :end");
    query.bindValue(":statusId", StatusId::Issued);
    query.bindValue(":start", startDate.toString(Qt::ISODate));
    query.bindValue(":end", endDate.toString(Qt::ISODate));

    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }
    return 0.0;
}

QList<QVariantMap> ReportBuilder::getMasterWorkload() {
    QList<QVariantMap> report;
    QSqlQuery query;
    // Позбулися магічних чисел
    query.prepare("SELECT e.last_name || ' ' || e.first_name AS master_name, COUNT(o.order_id) AS active_orders "
                  "FROM employees e "
                  "LEFT JOIN orders o ON e.employee_id = o.employee_id AND o.status_id IN (:s1, :s2, :s3) "
                  "GROUP BY e.employee_id "
                  "ORDER BY active_orders DESC");
    query.bindValue(":s1", StatusId::Accepted);
    query.bindValue(":s2", StatusId::InProgress);
    query.bindValue(":s3", StatusId::Ready);

    if (query.exec()) {
        while (query.next()) {
            QVariantMap row;
            row["master_name"] = query.value(0);
            row["active_orders"] = query.value(1);
            report.append(row);
        }
    }
    return report;
}