#include "OrderDialog.h"
#include "ClientManager.h"
#include "OrderStatusIds.h"
#include "OrderItemDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QSqlQuery>
#include <QMessageBox>
#include <QRegularExpressionValidator>
#include <QGroupBox>
#include <QLineEdit>
#include <QCompleter>
#include <QHeaderView>

OrderDialog::OrderDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Нове замовлення");
    resize(500, 400); // Робимо вікно достатньо просторим
    setupUi();
    
    populateClients();
    populateEmployees();
}

void OrderDialog::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // --- 1. ШАПКА ЗАМОВЛЕННЯ ---
    QGroupBox *headerGroup = new QGroupBox("Основна інформація", this);
    QFormLayout *form = new QFormLayout(headerGroup);

    QHBoxLayout *clientLayout = new QHBoxLayout();
    clientCombo = new QComboBox(this);
    clientCombo->setEditable(true);
    clientCombo->setInsertPolicy(QComboBox::NoInsert);
    clientCombo->completer()->setCompletionMode(QCompleter::PopupCompletion);

    btnAddClient = new QPushButton("➕", this);
    clientLayout->addWidget(clientCombo, 1);
    clientLayout->addWidget(btnAddClient);
    form->addRow("Клієнт *:", clientLayout);

    employeeCombo = new QComboBox(this);
    form->addRow("Майстер *:", employeeCombo);

    receivedDateEdit = new QDateEdit(QDate::currentDate(), this);
    receivedDateEdit->setCalendarPopup(true);
    requiredDateEdit = new QDateEdit(QDate::currentDate().addDays(3), this);
    requiredDateEdit->setCalendarPopup(true);
    form->addRow("Дата прийому *:", receivedDateEdit);
    form->addRow("Орієнтовна видача *:", requiredDateEdit);

    depositSpin = new QDoubleSpinBox(this);
    depositSpin->setMaximum(100000.0);
    form->addRow("Завдаток (грн):", depositSpin);

    notesEdit = new QTextEdit(this);
    notesEdit->setMaximumHeight(50);
    form->addRow("Нотатки:", notesEdit);
    mainLayout->addWidget(headerGroup);

    // --- 2. ТАБЛИЦЯ ПОЗИЦІЙ (НОВЕ) ---
    QGroupBox *itemsGroup = new QGroupBox("Послуги", this);
    QVBoxLayout *itemsLayout = new QVBoxLayout(itemsGroup);

    QHBoxLayout *itemsToolbar = new QHBoxLayout();
    QPushButton *btnAddItem = new QPushButton("Додати послугу", this);
    QPushButton *btnRemoveItem = new QPushButton("Видалити вибране", this);
    itemsToolbar->addWidget(btnAddItem);
    itemsToolbar->addWidget(btnRemoveItem);
    itemsToolbar->addStretch();
    itemsLayout->addLayout(itemsToolbar);

    itemsTable = new QTableView(this);
    itemsModel = new QStandardItemModel(0, 5, this);
    itemsModel->setHorizontalHeaderLabels({"Service ID", "Послуга", "Опис", "Кіл-сть", "Ціна (грн)", "Сума (грн)"});
    itemsTable->setModel(itemsModel);
    itemsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    itemsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    itemsTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // Редагування тільки через видалення/додавання
    itemsTable->horizontalHeader()->setStretchLastSection(true);
    itemsTable->hideColumn(0); // Ховаємо ID
    itemsLayout->addWidget(itemsTable);

    lblTotalSum = new QLabel("<b>Загальна сума: 0.00 грн</b>", this);
    lblTotalSum->setAlignment(Qt::AlignRight);
    itemsLayout->addWidget(lblTotalSum);

    mainLayout->addWidget(itemsGroup);

    // --- 3. КНОПКИ ---
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(buttons);

    // Підключення сигналів
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(btnAddClient, &QPushButton::clicked, this, &OrderDialog::onAddClientClicked);
    connect(btnAddItem, &QPushButton::clicked, this, &OrderDialog::onAddItemClicked);
    connect(btnRemoveItem, &QPushButton::clicked, this, &OrderDialog::onRemoveItemClicked);
}

void OrderDialog::onAddItemClicked() {
    OrderItemDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        if (dlg.getServiceId() <= 0) {
            QMessageBox::warning(this, "Помилка", "Оберіть послугу!");
            return;
        }

        double sum = dlg.getQuantity() * dlg.getUnitPrice();

        QList<QStandardItem *> rowItems;
        rowItems << new QStandardItem(QString::number(dlg.getServiceId()));
        rowItems << new QStandardItem(dlg.getServiceName());
        rowItems << new QStandardItem(dlg.getDescription());
        rowItems << new QStandardItem(QString::number(dlg.getQuantity()));
        rowItems << new QStandardItem(QString::number(dlg.getUnitPrice(), 'f', 2));
        rowItems << new QStandardItem(QString::number(sum, 'f', 2));

        itemsModel->appendRow(rowItems);
        calculateTotal();
    }
}

void OrderDialog::onRemoveItemClicked() {
    QModelIndex idx = itemsTable->currentIndex();
    if (idx.isValid()) {
        itemsModel->removeRow(idx.row());
        calculateTotal();
    }
}

void OrderDialog::calculateTotal() {
    double total = 0.0;
    for (int i = 0; i < itemsModel->rowCount(); ++i) {
        // Беремо значення з 5-ї колонки ("Сума")
        total += itemsModel->item(i, 5)->text().toDouble();
    }
    lblTotalSum->setText(QString("<b>Загальна сума: %1 грн</b>").arg(total, 0, 'f', 2));
}

std::vector<OrderItem> OrderDialog::getOrderItemsData() const {
    std::vector<OrderItem> items;
    for (int i = 0; i < itemsModel->rowCount(); ++i) {
        OrderItem item;
        item.serviceId = itemsModel->item(i, 0)->text().toInt();
        item.clothingDescription = itemsModel->item(i, 2)->text();
        item.quantity = itemsModel->item(i, 3)->text().toInt();
        item.unitPrice = itemsModel->item(i, 4)->text().toDouble();
        items.push_back(item);
    }
    return items;
}

void OrderDialog::populateClients() {
    clientCombo->clear();
    QSqlQuery q("SELECT client_id, last_name || ' ' || first_name || ' (' || phone || ')' FROM clients ORDER BY last_name");
    while (q.next()) {
        clientCombo->addItem(q.value(1).toString(), q.value(0).toInt());
    }
    clientCombo->setCurrentIndex(-1); // Щоб поле було порожнім спочатку
}

void OrderDialog::populateEmployees() {
    employeeCombo->clear();
    QSqlQuery q("SELECT employee_id, last_name || ' ' || first_name FROM employees ORDER BY last_name");
    while (q.next()) {
        employeeCombo->addItem(q.value(1).toString(), q.value(0).toInt());
    }
}

void OrderDialog::onAddClientClicked() {
    // Швидке діалогове вікно для створення клієнта
    QDialog dlg(this);
    dlg.setWindowTitle("Швидке додавання клієнта");
    QFormLayout form(&dlg);

    QLineEdit lnEdit, fnEdit, phoneEdit;
    phoneEdit.setPlaceholderText("+380XXXXXXXXX");
    QRegularExpression phoneRx("^\\+380\\d{9}$");
    phoneEdit.setValidator(new QRegularExpressionValidator(phoneRx, &dlg));

    form.addRow("Прізвище *:", &lnEdit);
    form.addRow("Ім'я *:", &fnEdit);
    form.addRow("Телефон *:", &phoneEdit);

    QDialogButtonBox btns(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dlg);
    form.addRow(&btns);
    connect(&btns, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(&btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted) {
        if (lnEdit.text().trimmed().isEmpty() || fnEdit.text().trimmed().isEmpty() || phoneEdit.text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Помилка", "Заповніть усі поля!");
            return;
        }

        Client newClient;
        newClient.lastName = lnEdit.text().trimmed();
        newClient.firstName = fnEdit.text().trimmed();
        newClient.phone = phoneEdit.text().trimmed();

        ClientManager cm;
        if (cm.registerClient(newClient)) {
            // Оновлюємо список клієнтів
            populateClients();
            // Знаходимо щойно доданого клієнта і вибираємо його
            int idx = clientCombo->findText(newClient.lastName + " " + newClient.firstName + " (" + newClient.phone + ")");
            if (idx >= 0) clientCombo->setCurrentIndex(idx);
        } else {
            QMessageBox::critical(this, "Помилка", "Клієнт із таким телефоном вже існує!");
        }
    }
}

Order OrderDialog::getOrderData() const {
    Order order;
    order.clientId = clientCombo->currentData().toInt();
    order.employeeId = employeeCombo->currentData().toInt();
    order.statusId = StatusId::Accepted; // Завжди "Прийнято" при створенні
    order.receivedDate = receivedDateEdit->date();
    order.requiredDate = requiredDateEdit->date();
    order.depositAmount = depositSpin->value();
    order.paymentStatus = (order.depositAmount > 0) ? "Частково" : "Неоплачено";
    order.notes = notesEdit->toPlainText().trimmed();
    return order;
}