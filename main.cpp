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
#include "src/ui/MainWindow.h"

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char *argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    QApplication app(argc, argv);
    if (!DatabaseManager::instance().init()) return -1;

    // Запускаємо НАШЕ головне вікно
    MainWindow mainWindow;
    mainWindow.show();

    int exitCode = app.exec();
    DatabaseManager::instance().close();
    return exitCode;
}