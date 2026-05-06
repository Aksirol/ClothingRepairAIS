#include <QApplication>
#include <QMainWindow>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>

#include "DatabaseManager.h"
#include "OrderStatusIds.h"
#include "OrderRepository.h"
#include "OrderManager.h"
#include "ClientManager.h"
#include "ReportBuilder.h"

#ifdef _WIN32
#include <windows.h>
#endif

void runBusinessLogicTests() {
    qDebug() << "\n==============================================";
    qDebug() << "=== ФАЗА 3: ТЕСТУВАННЯ БІЗНЕС-ЛОГІКИ (ФІНАЛ) ===";
    qDebug() << "==============================================\n";

    OrderManager orderManager;
    OrderRepository orderRepo;
    ClientManager clientManager;

    QSqlQuery seedQuery;
    seedQuery.exec("INSERT OR IGNORE INTO clients (client_id, last_name, first_name, phone) VALUES (99, 'Тест', 'Клієнт', '000')");
    seedQuery.exec("INSERT OR IGNORE INTO employees (employee_id, position_id, last_name, first_name, phone, hire_date) VALUES (99, 1, 'Майстер', 'Тест', '000', '2023-01-01')");
    seedQuery.exec("INSERT OR IGNORE INTO repair_services (service_id, category_id, service_name, base_price, estimated_days) VALUES (98, 1, 'Штани', 100.0, 1)");
    seedQuery.exec("INSERT OR IGNORE INTO repair_services (service_id, category_id, service_name, base_price, estimated_days) VALUES (99, 1, 'Куртка', 150.0, 1)");

    // --- ТЕСТ 1: ClientManager (Дублікати та Пошук) ---
    Client duplicateClient{-1, "Новий", "Клієнт", "", "000", "", "", QDate::currentDate()};
    if (!clientManager.registerClient(duplicateClient)) {
        qDebug() << "[+] ClientManager: Успішно заблоковано реєстрацію клієнта з існуючим телефоном.";
    }

    auto searchResults = clientManager.searchClients("000");
    if (!searchResults.empty() && searchResults.front().lastName == "Тест") {
        qDebug() << "[+] ClientManager: Пошук працює коректно.";
    }

    // --- ТЕСТ 2: OrderManager (Порожнє замовлення - ВІДНОВЛЕНО) ---
    Order emptyOrder{-1, 99, 99, StatusId::Accepted, QDate::currentDate(), QDate::currentDate().addDays(1), QDate(), 0.0, "Неоплачено", ""};
    std::vector<OrderItem> emptyItems;
    if (!orderManager.createOrderWithItems(emptyOrder, emptyItems)) {
        qDebug() << "[+] OrderManager: Успішно відхилено створення замовлення без позицій.";
    }

    // --- ТЕСТ 3: OrderManager (Транзакція / Обрив) ---
    int ordersCountBefore = orderRepo.getAll().size();
    Order rollbackOrder{-1, 99, 99, StatusId::Accepted, QDate::currentDate(), QDate::currentDate().addDays(1), QDate(), 0.0, "Неоплачено", ""};
    std::vector<OrderItem> rollbackItems = {
        OrderItem{-1, -1, 98, 1, 0.0, "Одяг 1", ""},
        OrderItem{-1, -1, 9999, 1, 0.0, "Неіснуюча", ""}
    };
    if (!orderManager.createOrderWithItems(rollbackOrder, rollbackItems)) {
        if (ordersCountBefore == orderRepo.getAll().size()) {
            qDebug() << "[+] OrderManager: Транзакцію успішно відкочено!";
        }
    }

    // --- ТЕСТ 4: OrderManager (Незаконний перехід статусу - ВІДНОВЛЕНО) ---
    Order issuedOrder{-1, 99, 99, StatusId::Issued, QDate::currentDate(), QDate::currentDate(), QDate::currentDate(), 0.0, "Оплачено", ""};
    int testOrderId = orderRepo.insert(issuedOrder);
    if (!orderManager.changeOrderStatus(testOrderId, StatusId::Accepted)) {
         qDebug() << "[+] OrderManager: Успішно заблоковано незаконний перехід зі статусу 'Видано' назад у 'Прийнято'.";
    }

    // --- ТЕСТ 5: ReportBuilder (Виторг за order_items) ---
    QDate today = QDate::currentDate();
    Order o1{-1, 99, 99, StatusId::Issued, today, today, today, 50.0, "Оплачено", ""};
    std::vector<OrderItem> items1 = { OrderItem{-1, -1, 98, 2, 0.0, "", ""} }; // 2 * 100 = 200
    orderManager.createOrderWithItems(o1, items1);

    Order o2{-1, 99, 99, StatusId::Issued, today, today, today, 0.0, "Оплачено", ""};
    std::vector<OrderItem> items2 = { OrderItem{-1, -1, 99, 1, 0.0, "", ""} }; // 1 * 150 = 150
    orderManager.createOrderWithItems(o2, items2);

    Order o3{-1, 99, 99, StatusId::InProgress, today, today, QDate(), 0.0, "Неоплачено", ""};
    std::vector<OrderItem> items3 = { OrderItem{-1, -1, 99, 10, 0.0, "", ""} }; // Не має враховуватись
    orderManager.createOrderWithItems(o3, items3);

    double revenue = ReportBuilder::getRevenueForPeriod(today, today);
    if (qFuzzyCompare(revenue, 350.0)) {
         qDebug() << "[+] ReportBuilder: Ідеально! Виторг за сьогодні склав" << revenue << "(Очікувалось 350).";
    }

    qDebug() << "==============================================\n";
}

int main(int argc, char *argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    QApplication app(argc, argv);
    if (!DatabaseManager::instance().init()) return -1;

    runBusinessLogicTests();

    QMainWindow mainWindow;
    mainWindow.setWindowTitle("АІС: Ремонт одягу"); // Відновлено!
    mainWindow.resize(800, 600);
    mainWindow.show();

    int exitCode = app.exec();
    DatabaseManager::instance().close();
    return exitCode;
}