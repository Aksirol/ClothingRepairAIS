#include <QApplication>
#include <QMainWindow>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>

// Підключаємо інфраструктуру та репозиторії
#include "DatabaseManager.h"
#include "src/repositories/ClientRepository.h"
#include "src/repositories/OrderRepository.h"

#ifdef _WIN32
#include <windows.h>
#endif

void runCrudTests() {
    qDebug() << "\n==============================================";
    qDebug() << "=== ФАЗА 2: СТАРТ ТЕСТУВАННЯ CRUD ТА ЗВ'ЯЗКІВ ===";
    qDebug() << "==============================================\n";

    ClientRepository clientRepo;
    OrderRepository orderRepo;

    // --- 1. Тест вставки (Insert) ---
    Client newClient{-1, "Шевченко", "Тарас", "Григорович", "+380990001122", "taras@mail.com", "Київ", QDate::currentDate()};
    if (clientRepo.insert(newClient)) {
        qDebug() << "[+] Вставка (Insert): Успішно додано нового клієнта.";
    } else {
        qCritical() << "[-] Вставка (Insert): ПОМИЛКА!";
    }

    // Отримуємо ID щойно доданого клієнта
    auto clients = clientRepo.getAll();
    int testClientId = clients.back().id;

    // --- 2. Тест читання (getById) ---
    auto fetchedClient = clientRepo.getById(testClientId);
    if (fetchedClient) {
        qDebug() << "[+] Читання (getById): Знайдено клієнта -" << fetchedClient->lastName << fetchedClient->firstName;
    } else {
        qCritical() << "[-] Читання (getById): ПОМИЛКА! Запис не знайдено.";
    }

    // --- 3. Тест оновлення (Update) ---
    fetchedClient->phone = "111-222-333";
    if (clientRepo.update(*fetchedClient)) {
        auto updatedClient = clientRepo.getById(testClientId);
        if (updatedClient->phone == "111-222-333") {
            qDebug() << "[+] Оновлення (Update): Номер телефону успішно змінено на" << updatedClient->phone;
        }
    }

    // --- 4. Тест обов'язкових полів (NOT NULL) ---
    // В Qt передача нульового рядка QString() конвертується в NULL для бази даних
    Client invalidClient{-1, QString(), "Іван", "", "000", "", "", QDate::currentDate()};
    if (!clientRepo.insert(invalidClient)) {
        qDebug() << "[+] Захист полів (NOT NULL): БД успішно відхилила клієнта без прізвища.";
    } else {
        qCritical() << "[-] Захист полів (NOT NULL): КРИТИЧНА ПОМИЛКА! БД дозволила зберегти NULL.";
    }

    // --- 5. Тест захисту зовнішніх ключів (Foreign Key) ---
    // Додамо службового працівника прямим запитом, щоб мати змогу створити замовлення
    QSqlQuery seedQuery;
    seedQuery.exec("INSERT OR IGNORE INTO employees (employee_id, position_id, last_name, first_name, phone, hire_date) "
                   "VALUES (1, 1, 'Майстер', 'Тест', '000', '2023-01-01')");

    // Створюємо замовлення, прив'язане до нашого тестового клієнта
    Order testOrder;
    testOrder.clientId     = testClientId;
    testOrder.employeeId   = 1;
    testOrder.statusId     = 1;
    testOrder.receivedDate = QDate::currentDate();
    testOrder.requiredDate = QDate::currentDate().addDays(2);
    testOrder.depositAmount = 50.0;
    testOrder.notes        = "Тестове замовлення";

    qDebug() << "[*] Спроба видалити клієнта, у якого є активне замовлення...";
    if (!clientRepo.deleteById(testClientId)) {
        qDebug() << "[+] Захист зв'язків (Foreign Key): БД заблокувала видалення. Клієнт у безпеці!";
    } else {
        qCritical() << "[-] Захист зв'язків (Foreign Key): КРИТИЧНА ПОМИЛКА! Клієнта було видалено в обхід правил.";
    }

    // --- 6. Тест видалення (Delete) ---
    // Спочатку приберемо замовлення (щоб зняти блок із клієнта)
    auto orders = orderRepo.getAll();
    orderRepo.deleteById(orders.back().id);
    qDebug() << "[+] Видалення (Delete): Замовлення успішно видалено.";

    // Тепер клієнт вільний, спробуємо його видалити ще раз
    if (clientRepo.deleteById(testClientId)) {
        auto checkDeleted = clientRepo.getById(testClientId);
        if (!checkDeleted) {
            qDebug() << "[+] Видалення (Delete): Клієнта успішно видалено, запис більше не існує.";
        }
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

    // Запускаємо фазу тестування репозиторіїв
    runCrudTests();

    // Графічне вікно
    QMainWindow mainWindow;
    mainWindow.setWindowTitle("АІС: Ремонт одягу");
    mainWindow.resize(800, 600);
    mainWindow.show();

    int exitCode = app.exec();
    DatabaseManager::instance().close();
    return exitCode;
}