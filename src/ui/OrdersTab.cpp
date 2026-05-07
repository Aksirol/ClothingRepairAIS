#include "OrdersTab.h"
#include "OrderStatusDelegate.h"
#include "OrderStatusIds.h"
#include "OrderManager.h"
#include "OrderDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QSqlQuery>
#include <QSqlError>
#include <QDate>
#include <QMessageBox>

OrdersTab::OrdersTab(QWidget *parent) : QWidget(parent) {
    setupUi();
    setupModel();
}

void OrdersTab::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // --- Верхня панель інструментів ---
    QHBoxLayout *toolbarLayout = new QHBoxLayout();
    
    QPushButton *btnAdd = new QPushButton("➕ Створити замовлення", this);
    QPushButton *btnEdit = new QPushButton("✏️ Редагувати", this);
    QPushButton *btnDelete = new QPushButton("❌ Видалити", this);
    
    // Фільтр за статусами
    statusFilterCombo = new QComboBox(this);
    statusFilterCombo->addItem("Всі статуси", "");
    // Заповнимо решту при оновленні або вручну. Для швидкості запишемо зараз:
    QSqlQuery sq("SELECT status_name FROM order_statuses ORDER BY status_id");
    while(sq.next()) { statusFilterCombo->addItem(sq.value(0).toString(), sq.value(0).toString()); }

    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("Пошук (Номер, ПІБ клієнта)...");
    searchEdit->setMinimumWidth(200);

    toolbarLayout->addWidget(btnAdd);
    toolbarLayout->addWidget(btnEdit);
    toolbarLayout->addWidget(btnDelete);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(statusFilterCombo);
    toolbarLayout->addWidget(searchEdit);

    mainLayout->addLayout(toolbarLayout);

    // --- Таблиця ---
    tableView = new QTableView(this);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setSortingEnabled(true);
    tableView->horizontalHeader()->setStretchLastSection(true);
    
    // Встановлюємо наш кольоровий делегат!
    tableView->setItemDelegate(new OrderStatusDelegate(colorColumnIndex, this));

    mainLayout->addWidget(tableView);

    // --- Сигнали ---
    connect(btnAdd, &QPushButton::clicked, this, &OrdersTab::onAddOrderClicked);
    connect(btnEdit, &QPushButton::clicked, this, &OrdersTab::onEditOrderClicked);
    connect(btnDelete, &QPushButton::clicked, this, &OrdersTab::onDeleteOrderClicked);
    connect(searchEdit, &QLineEdit::textChanged, this, &OrdersTab::onSearchTextChanged);
    connect(statusFilterCombo, &QComboBox::currentIndexChanged, this, &OrdersTab::onStatusFilterChanged);
}

void OrdersTab::setupModel() {
    model = new QSqlQueryModel(this);
    
    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterKeyColumn(-1); // Пошук скрізь

    tableView->setModel(proxyModel);
    
    refreshData();
}

void OrdersTab::refreshData() {
    QSqlQuery query;
    // Отримуємо зведені дані. Зверни увагу на логіку обчислення row_color!
    query.prepare(
        "SELECT "
        "  o.order_id AS 'Номер', "
        "  c.last_name || ' ' || c.first_name AS 'Клієнт', "
        "  e.last_name AS 'Майстер', "
        "  s.status_name AS 'Статус', "
        "  o.received_date AS 'Прийнято', "
        "  o.required_date AS 'Дедлайн', "
        "  o.deposit_amount AS 'Завдаток', "
        "  o.payment_status AS 'Оплата', "
        "  CASE "
        "    WHEN o.required_date < :today AND o.status_id NOT IN (:issued, :cancelled) THEN '#FF0000' " // Червоний для прострочених
        "    ELSE s.color_code " // Стандартний колір з БД
        "  END AS row_color "
        "FROM orders o "
        "JOIN clients c ON o.client_id = c.client_id "
        "JOIN employees e ON o.employee_id = e.employee_id "
        "JOIN order_statuses s ON o.status_id = s.status_id"
    );
    
    query.bindValue(":today", QDate::currentDate().toString(Qt::ISODate));
    query.bindValue(":issued", StatusId::Issued);
    query.bindValue(":cancelled", StatusId::Cancelled);
    query.exec();

    model->setQuery(query); // Передаємо виконаний запит у модель

    // Приховуємо технічну колонку з кольором
    tableView->hideColumn(colorColumnIndex);
}

void OrdersTab::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    refreshData(); // Завжди свіжі дані при відкритті вкладки
}

void OrdersTab::onSearchTextChanged(const QString &text) {
    // QSortFilterProxyModel сам вміє фільтрувати по тексту
    proxyModel->setFilterRegularExpression(text);
}

void OrdersTab::onStatusFilterChanged(int index) {
    // Якщо вибрано "Всі статуси", скидаємо фільтр. Інакше шукаємо точний збіг назви статусу.
    QString status = statusFilterCombo->currentData().toString();
    if (status.isEmpty()) {
        proxyModel->setFilterRegularExpression("");
    } else {
        proxyModel->setFilterKeyColumn(3); // Колонка 'Статус'
        proxyModel->setFilterRegularExpression("^" + status + "$"); // Точний збіг
    }
}

// Заглушки для Під-етапів 7.2 та 7.3
void OrdersTab::onAddOrderClicked() {
    OrderDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        Order newOrder = dialog.getOrderData();
        std::vector<OrderItem> items = dialog.getOrderItemsData();

        if (newOrder.clientId <= 0 || newOrder.employeeId <= 0) {
            QMessageBox::warning(this, "Помилка", "Необхідно вибрати клієнта та майстра!");
            return;
        }
        if (items.empty()) {
            QMessageBox::warning(this, "Помилка", "Замовлення повинно містити хоча б одну послугу!");
            return;
        }

        // ТРАНЗАКЦІЙНЕ ЗБЕРЕЖЕННЯ ЧЕРЕЗ МЕНЕДЖЕР
        OrderManager om;
        if (om.createOrderWithItems(newOrder, items)) {
            refreshData();
            QMessageBox::information(this, "Успіх", "Замовлення успішно створено!");
        } else {
            QMessageBox::critical(this, "Помилка", "Не вдалося зберегти замовлення. Транзакцію відхилено.");
        }
    }
}

void OrdersTab::onEditOrderClicked() {
    QMessageBox::information(this, "Фаза 7.3", "Тут буде редагування замовлення та додавання позицій.");
}

void OrdersTab::onDeleteOrderClicked() {
    QMessageBox::information(this, "Видалення", "Виклик OrderRepository::deleteById(id)");
}