#include "MainWindow.h"
#include "ReferencesTab.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QFont>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("АІС: Ремонт одягу");
    resize(1024, 768); // Робимо вікно трохи більшим для комфортної роботи
    
    setupUi();
}

void MainWindow::setupUi() {
    // Центральний віджет, який займе весь простір вікна
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Головний горизонтальний шар (зліва меню, справа контент)
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0); // Прибираємо відступи по краях
    mainLayout->setSpacing(0);

    // 1. Бічна панель (Меню)
    sidebar = new QListWidget(this);
    sidebar->setMaximumWidth(220);
    // Додаємо трохи стилізації через CSS-подібний синтаксис Qt (QSS)
    sidebar->setStyleSheet(
        "QListWidget {"
        "   background-color: #f0f0f0;"
        "   border: none;"
        "   border-right: 1px solid #cccccc;"
        "   padding-top: 10px;"
        "   color: #222222;" /* Явно задаємо темний колір тексту для всієї панелі */
        "}"
        "QListWidget::item {"
        "   padding: 10px;"
        "   font-size: 14px;"
        "   color: #222222;" /* Темний колір для неактивних пунктів */
        "}"
        "QListWidget::item:hover {"
        "   background-color: #e0e0e0;" /* Світло-сірий фон при наведенні мишки */
        "}"
        "QListWidget::item:selected {"
        "   background-color: #0078D7;" /* Синій фон для вибраного пункту */
        "   color: white;"              /* Білий текст для вибраного пункту */
        "}"
    );

    // Додаємо пункти меню
    sidebar->addItem("📦 Замовлення");
    sidebar->addItem("👥 Клієнти");
    sidebar->addItem("🛠 Послуги");
    sidebar->addItem("📚 Довідники");
    sidebar->addItem("📊 Звіти");

    // 2. Робоча область (Стопки віджетів)
    stackedWidget = new QStackedWidget(this);

    // Збираємо все докупи
    mainLayout->addWidget(sidebar);
    mainLayout->addWidget(stackedWidget);

    // Створюємо самі сторінки-заглушки
    createPages();

    // 3. Підключення логіки навігації
    // Коли змінюється рядок у меню -> змінюється сторінка у StackedWidget
    connect(sidebar, &QListWidget::currentRowChanged, stackedWidget, &QStackedWidget::setCurrentIndex);

    // За замовчуванням виділяємо перший пункт (Замовлення)
    sidebar->setCurrentRow(0);
}

void MainWindow::createPages() {
    // 0. Замовлення (Заглушка)
    stackedWidget->addWidget(createDummyPage("Управління замовленнями"));

    // 1. Клієнти (Заглушка)
    stackedWidget->addWidget(createDummyPage("База клієнтів"));

    // 2. Послуги (Заглушка)
    stackedWidget->addWidget(createDummyPage("Каталог послуг"));

    // 3. Довідники (Контейнер з вкладками)
    ReferencesTab *referencesTab = new ReferencesTab(this);
    stackedWidget->addWidget(referencesTab);

    // 4. Звіти (Заглушка)
    stackedWidget->addWidget(createDummyPage("Звіти та Аналітика"));
}

QWidget* MainWindow::createDummyPage(const QString& title) {
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    QLabel *label = new QLabel(title, page);
    QFont font = label->font();
    font.setPointSize(24);
    font.setBold(true);
    label->setFont(font);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("color: #666666;");
    layout->addWidget(label);
    return page;
}