#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QStackedWidget>

class MainWindow : public QMainWindow {
    Q_OBJECT // Обов'язковий макрос для всіх класів, що працюють із сигналами/слотами Qt

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;

private:
    void setupUi();      // Метод для налаштування загального вигляду
    void createPages();  // Метод для генерації порожніх заглушок сторінок
    QWidget *createDummyPage(const QString &title);

    QListWidget *sidebar;
    QStackedWidget *stackedWidget;
};

#endif // MAINWINDOW_H