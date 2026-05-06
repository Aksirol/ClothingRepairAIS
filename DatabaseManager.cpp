#include "DatabaseManager.h"
#include <QCoreApplication>
#include <QSqlError>
#include <QSqlQuery>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QFileInfo>

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::~DatabaseManager() {

}

bool DatabaseManager::init() {
    QString appDir = QCoreApplication::applicationDirPath();
    QString dbPath = appDir + QDir::separator() + "repairs.db";

    // Перевіряємо, чи маємо ми право писати в папку поруч з .exe
    QFileInfo appDirInfo(appDir);
    if (!appDirInfo.isWritable()) {
        qWarning() << "Папка додатку недоступна для запису. Використовуємо AppData.";
        // Шлях: C:\Users\Username\AppData\Local\ClothingRepairAIS\repairs.db
        QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
        QDir().mkpath(dataDir); // Створюємо папку, якщо її немає
        dbPath = dataDir + QDir::separator() + "repairs.db";
    }

    qDebug() << "Шлях до бази даних:" << dbPath;

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        qCritical() << "Помилка відкриття бази даних:" << db.lastError().text();
        return false;
    }

    QSqlQuery pragmaQuery(db);
    pragmaQuery.exec("PRAGMA foreign_keys = ON;");

    if (!db.tables().contains("db_version")) {
        qDebug() << "База даних порожня. Запускаємо міграцію схеми...";
        if (!executeSchema()) {
            return false;
        }
    } else {
        qDebug() << "База даних вже існує. Міграція не потрібна.";
    }

    return true;
}

bool DatabaseManager::executeSchema() {
    // Зчитуємо файл із ресурсів (зверни увагу на префікс ':/')
    QFile schemaFile(":/schema.sql");
    if (!schemaFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "Не вдалося знайти файл schema.sql у ресурсах!";
        return false;
    }

    QTextStream in(&schemaFile);
    QString sql = in.readAll();
    schemaFile.close();

    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);

    // ВАЖЛИВО: QSqlQuery::exec() в SQLite не вміє виконувати багато запитів за один раз.
    // Тому ми розбиваємо весь текст на окремі запити по крапці з комою (;).
    QStringList queries = sql.split(";", Qt::SkipEmptyParts);

    for (QString q : queries) {
        q = q.trimmed();
        if (q.isEmpty()) continue;

        if (!query.exec(q)) {
            qCritical() << "Помилка виконання SQL-запиту:" << query.lastError().text();
            qCritical() << "Запит:" << q;
            return false;
        }
    }

    qDebug() << "Схему успішно застосовано, початкові дані завантажено!";
    return true;
}

QSqlDatabase DatabaseManager::getDatabase() const {
    return QSqlDatabase::database();
}

void DatabaseManager::close() {
    QSqlDatabase db = QSqlDatabase::database();
    if (db.isOpen()) {
        db.close();
    }
}