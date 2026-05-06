#ifndef REPORTBUILDER_H
#define REPORTBUILDER_H

#include <QList>
#include <QVariantMap>
#include <QDate>

class ReportBuilder {
public:
    // Активні замовлення (статуси: Прийнято, В роботі)
    static QList<QVariantMap> getActiveOrders();

    // Прострочені замовлення (required_date < сьогодні, і ще не видані)
    static QList<QVariantMap> getOverdueOrders();

    // Виторг за вказаний період
    static double getRevenueForPeriod(const QDate& startDate, const QDate& endDate);

    // Завантаженість майстрів (хто скільки замовлень зараз виконує)
    static QList<QVariantMap> getMasterWorkload();
};

#endif