#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QString>

class DatabaseManager {
public:
    // Повертає єдиний екземпляр класу (Singleton)
    static DatabaseManager& instance();

    // Головний метод ініціалізації
    bool init();

    // Отримання об'єкта підключення до БД
    QSqlDatabase getDatabase() const;

    // Закриття підключення
    void close();

private:
    // Приватний конструктор, щоб заборонити створення об'єктів ззовні
    DatabaseManager() = default;
    ~DatabaseManager();

    // Забороняємо копіювання (правило для Singleton)
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    // Допоміжний метод для виконання SQL-схеми
    bool executeSchema();
};

#endif // DATABASEMANAGER_H