#include <QApplication>
#include <QMainWindow>
#include <QDebug>
#include "DatabaseManager.h" // Підключаємо наш менеджер

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Ініціалізуємо базу даних
    if (!DatabaseManager::instance().init()) {
        qCritical() << "Критична помилка: не вдалося ініціалізувати базу даних!";
        return -1; // Зупиняємо програму, якщо база не працює
    }

    QMainWindow mainWindow;
    mainWindow.setWindowTitle("АІС: Ремонт одягу");
    mainWindow.resize(800, 600);

    mainWindow.show();

    return app.exec();
}