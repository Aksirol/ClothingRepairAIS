#include <QApplication>
#include <QMainWindow>
#include <QDebug>

int main(int argc, char *argv[]) {
    // QApplication керує загальними налаштуваннями додатку та подіями
    QApplication app(argc, argv);

    // Створюємо головне вікно
    QMainWindow mainWindow;
    mainWindow.setWindowTitle("АІС: Ремонт одягу");
    mainWindow.resize(800, 600);

    qDebug() << "Додаток успішно запущено!";

    // Показуємо вікно на екрані
    mainWindow.show();

    // Запускаємо головний цикл обробки подій
    return app.exec();
}