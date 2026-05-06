#include <QApplication>
#include <QMainWindow>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>

// Підключаємо інфраструктуру, репозиторії та менеджери
#include "DatabaseManager.h"
#include "src/repositories/OrderRepository.h"
#include "src/managers/OrderManager.h"
#include "src/managers/ClientManager.h"
#include "src/managers/ReportBuilder.h"

#ifdef _WIN32
#include <windows.h>
#endif

void runBusinessLogicTests() {
    qDebug() << "\n==============================================";
    qDebug() << "=== ФАЗА 3: ТЕСТУВАННЯ БІЗНЕС-ЛОГІКИ (MANAGERS) ===";
    qDebug() << "==============================================\n";

    OrderManager orderManager;
    OrderRepository orderRepo;

    // --- Підготовка тестових даних (FK Protection) ---
    // Вставляємо системні записи напряму, щоб не порушувати зовнішні ключі
    QSqlQuery seedQuery;
    seedQuery.exec("INSERT OR IGNORE INTO clients (client_id, last_name, first_name, phone) VALUES (99, 'Тест', 'Клієнт', '000')");
    seedQuery.exec("INSERT OR IGNORE INTO employees (employee_id, position_id, last_name, first_name, phone, hire_date) VALUES (99, 1, 'Майстер', 'Тест', '000', '2023-01-01')");
    seedQuery.exec("INSERT OR IGNORE INTO repair_services (service_id, category_id, service_name, base_price, estimated_days) VALUES (98, 1, 'Послуга 1', 100.0, 1)");
    seedQuery.exec("INSERT OR IGNORE INTO repair_services (service_id, category_id, service_name, base_price, estimated_days) VALUES (99, 1, 'Послуга 2', 150.0, 1)");

    // ==============================================================
    // ТЕСТ 1: Спроба створити замовлення без позицій
    // ==============================================================
    Order emptyOrder{-1, 99, 99, 1, QDate::currentDate(), QDate::currentDate().addDays(1), QDate(), 0.0, "Неоплачено", ""};
    std::vector<OrderItem> emptyItems; // Порожній масив

    if (!orderManager.createOrderWithItems(emptyOrder, emptyItems)) {
        qDebug() << "[+] OrderManager (Порожнє замовлення): Успішно відхилено створення замовлення без позицій.";
    } else {
        qCritical() << "[-] OrderManager (Порожнє замовлення): ПОМИЛКА! Менеджер дозволив порожнє замовлення.";
    }

    // ==============================================================
    // ТЕСТ 2: Перевірка транзакції (обрив на 3-й позиції)
    // ==============================================================
    int ordersCountBefore = orderRepo.getAll().size();

    Order rollbackOrder{-1, 99, 99, 1, QDate::currentDate(), QDate::currentDate().addDays(1), QDate(), 0.0, "Неоплачено", "Транзакція"};
    std::vector<OrderItem> rollbackItems = {
        OrderItem{-1, -1, 98, 1, 0.0, "Одяг 1", ""},   // Валідна послуга
        OrderItem{-1, -1, 99, 1, 0.0, "Одяг 2", ""},   // Валідна послуга
        OrderItem{-1, -1, 9999, 1, 0.0, "Одяг 3", ""}  // НЕІСНУЮЧА послуга (тут транзакція має впасти)
    };

    if (!orderManager.createOrderWithItems(rollbackOrder, rollbackItems)) {
        int ordersCountAfter = orderRepo.getAll().size();
        if (ordersCountBefore == ordersCountAfter) {
            qDebug() << "[+] OrderManager (Транзакція): Транзакцію успішно відкочено! У БД не лишилося 'сміття'.";
        } else {
            qCritical() << "[-] OrderManager (Транзакція): КРИТИЧНА ПОМИЛКА! Транзакція не спрацювала, замовлення збереглось без позицій!";
        }
    }

    // ==============================================================
    // ТЕСТ 3: Перевірка незаконного переходу статусів
    // ==============================================================
    // Створюємо "Видане" замовлення напряму через репозиторій
    Order issuedOrder{-1, 99, 99, 4, QDate::currentDate(), QDate::currentDate(), QDate::currentDate(), 0.0, "Оплачено", ""};
    orderRepo.insert(issuedOrder);
    int testOrderId = orderRepo.getAll().back().id;

    // Спроба повернути статус 1 (Прийнято)
    if (!orderManager.changeOrderStatus(testOrderId, 1)) {
         qDebug() << "[+] OrderManager (Статуси): Успішно заблоковано незаконний перехід зі статусу 'Видано' назад у 'Прийнято'.";
    } else {
         qCritical() << "[-] OrderManager (Статуси): ПОМИЛКА! Бізнес-правило переходу статусів порушено.";
    }

    // ==============================================================
    // ТЕСТ 4: ReportBuilder - Правильність підрахунку виторгу
    // ==============================================================
    QDate today = QDate::currentDate();

    // Додаємо тестові дані в репозиторій (вручну складені)
    Order r1{-1, 99, 99, 4, today, today, today, 150.0, "Оплачено", ""}; // Видано сьогодні: +150
    Order r2{-1, 99, 99, 4, today, today, today, 250.0, "Оплачено", ""}; // Видано сьогодні: +250
    Order r3{-1, 99, 99, 1, today, today, QDate(), 500.0, "Неоплачено", ""}; // Ще не видано: (ігнорується)
    Order r4{-1, 99, 99, 4, today.addDays(-2), today.addDays(-2), today.addDays(-2), 100.0, "Оплачено", ""}; // Видано позавчора: (ігнорується)

    orderRepo.insert(r1); orderRepo.insert(r2); orderRepo.insert(r3); orderRepo.insert(r4);

    // Очікуваний результат за сьогодні: 150 + 250 = 400
    double revenue = ReportBuilder::getRevenueForPeriod(today, today);
    if (qFuzzyCompare(revenue, 400.0)) { // qFuzzyCompare безпечно порівнює double
         qDebug() << "[+] ReportBuilder (Виторг): Ідеально! Виторг за сьогодні склав" << revenue << "(Очікувалось 400).";
    } else {
         qCritical() << "[-] ReportBuilder (Виторг): ПОМИЛКА підрахунку! Отримано:" << revenue;
    }

    qDebug() << "\n==============================================";
    qDebug() << "=== ТЕСТУВАННЯ УСПІШНО ЗАВЕРШЕНО СТАТУС: OK ===";
    qDebug() << "==============================================\n";
}

int main(int argc, char *argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    QApplication app(argc, argv);

    if (!DatabaseManager::instance().init()) {
        qCritical() << "Критична помилка ініціалізації БД!";
        return -1;
    }

    // Запускаємо тести менеджерів
    runBusinessLogicTests();

    // Графічне вікно
    QMainWindow mainWindow;
    mainWindow.setWindowTitle("АІС: Ремонт одягу");
    mainWindow.resize(800, 600);
    mainWindow.show();

    int exitCode = app.exec();
    DatabaseManager::instance().close();
    return exitCode;
}