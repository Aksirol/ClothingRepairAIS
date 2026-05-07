#include "OrdersTab.h"
#include "OrderStatusDelegate.h"
#include "OrderStatusIds.h"
#include "OrderManager.h"
#include "OrderDialog.h"
#include "OrderRepository.h"
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
    // Прибираємо setFilterKeyColumn і setFilterRegularExpression звідси!

    tableView->setModel(proxyModel);
    refreshData();
}

void OrdersTab::refreshData() {
    QString status = statusFilterCombo->currentData().toString();
    QString search = searchEdit->text().trimmed();

    QString queryString =
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
        "    WHEN o.required_date < :today AND o.status_id NOT IN (:issued, :cancelled) THEN '#FF0000' "
        "    ELSE s.color_code "
        "  END AS row_color "
        "FROM orders o "
        "JOIN clients c ON o.client_id = c.client_id "
        "JOIN employees e ON o.employee_id = e.employee_id "
        "JOIN order_statuses s ON o.status_id = s.status_id "
        "WHERE 1=1 "; // Базова умова для легкого приєднання AND

    // Динамічне додавання фільтрів до SQL
    if (!status.isEmpty()) {
        queryString += " AND s.status_name = :status ";
    }
    if (!search.isEmpty()) {
        queryString += " AND (CAST(o.order_id AS TEXT) LIKE :search OR c.last_name LIKE :search) ";
    }

    QSqlQuery query;
    query.prepare(queryString);
    query.bindValue(":today", QDate::currentDate().toString(Qt::ISODate));
    query.bindValue(":issued", StatusId::Issued);
    query.bindValue(":cancelled", StatusId::Cancelled);

    if (!status.isEmpty()) query.bindValue(":status", status);
    if (!search.isEmpty()) query.bindValue(":search", "%" + search + "%");

    query.exec();
    model->setQuery(query);
    tableView->hideColumn(colorColumnIndex);
}

void OrdersTab::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);

    // Зберігаємо поточний вибраний статус, щоб він не скидався при оновленні
    QString currentStatus = statusFilterCombo->currentData().toString();

    // Оновлюємо список статусів з БД
    statusFilterCombo->blockSignals(true); // Блокуємо сигнали, щоб не викликати refreshData() 10 разів
    statusFilterCombo->clear();
    statusFilterCombo->addItem("Всі статуси", "");
    QSqlQuery sq("SELECT status_name FROM order_statuses ORDER BY status_id");
    while(sq.next()) {
        statusFilterCombo->addItem(sq.value(0).toString(), sq.value(0).toString());
    }

    // Відновлюємо вибір
    int idx = statusFilterCombo->findData(currentStatus);
    if (idx >= 0) statusFilterCombo->setCurrentIndex(idx);
    statusFilterCombo->blockSignals(false);

    refreshData();
}

void OrdersTab::onSearchTextChanged(const QString &text) {
    refreshData(); // Фільтрує тепер SQL
}

void OrdersTab::onStatusFilterChanged(int index) {
    refreshData(); // Фільтрує тепер SQL
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
    QModelIndex proxyIndex = tableView->currentIndex();
    if (!proxyIndex.isValid()) {
        QMessageBox::warning(this, "Увага", "Виберіть замовлення для видалення.");
        return;
    }

    int row = proxyModel->mapToSource(proxyIndex).row();
    int orderId = model->index(row, 0).data().toInt();

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Підтвердження",
        "Ви впевнені, що хочете видалити це замовлення? Усі його послуги будуть також видалені.",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        OrderRepository repo;
        if (repo.deleteById(orderId)) {
            refreshData();
        } else {
            QMessageBox::critical(this, "Помилка", "Не вдалося видалити замовлення.");
        }
    }
}