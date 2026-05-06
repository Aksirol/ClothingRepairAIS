#include "OrderManager.h"
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
    
    // Починаємо транзакцію
    if (!db.transaction()) {
        qCritical() << "Не вдалося розпочати транзакцію!";
        return false;
    }

    // 1. Зберігаємо замовлення
    if (!orderRepo.insert(order)) {
        db.rollback();
        return false;
    }

    // 2. Отримуємо ID щойно створеного замовлення
    QSqlQuery query("SELECT last_insert_rowid()");
    int newOrderId = -1;
    if (query.next()) {
        newOrderId = query.value(0).toInt();
    } else {
        db.rollback();
        return false;
    }

    // 3. Зберігаємо всі позиції, прив'язуючи їх до нового замовлення
    for (auto& item : items) {
        item.orderId = newOrderId;
        if (!itemRepo.insert(item)) {
            qCritical() << "Помилка при збереженні позиції. Відкат транзакції.";
            db.rollback(); // Якщо хоча б одна позиція не збереглася — відкочуємо все
            return false;
        }
    }

    // Якщо все добре — підтверджуємо транзакцію
    return db.commit();
}

bool OrderManager::changeOrderStatus(int orderId, int newStatusId) {
    auto orderOpt = orderRepo.getById(orderId);
    if (!orderOpt) return false;
    
    Order order = *orderOpt;

    // Бізнес-правило: Зі статусу "Видано" (4) або "Скасовано" (5) повертатися не можна
    if (order.statusId == 4 || order.statusId == 5) {
        qWarning() << "Заборонено змінювати статус для виданих або скасованих замовлень.";
        return false;
    }

    order.statusId = newStatusId;

    // Бізнес-правило: Автоматично ставимо дату завершення, якщо статус "Готово" (3)
    if (newStatusId == 3) {
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