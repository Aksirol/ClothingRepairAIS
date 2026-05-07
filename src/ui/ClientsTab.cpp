#include "ClientsTab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlError>
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QRegularExpressionValidator>
#include <QSqlQuery>

ClientsTab::ClientsTab(QWidget *parent) : QWidget(parent) {
    setupUi();
    setupModel();
}

void ClientsTab::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *toolbarLayout = new QHBoxLayout();
    
    QPushButton *btnAdd = new QPushButton("Додати", this);
    QPushButton *btnEdit = new QPushButton("Редагувати", this);
    QPushButton *btnDelete = new QPushButton("Видалити", this);
    QLineEdit *searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("Пошук клієнта (ПІБ, телефон)...");
    searchEdit->setMinimumWidth(250);

    toolbarLayout->addWidget(btnAdd);
    toolbarLayout->addWidget(btnEdit);
    toolbarLayout->addWidget(btnDelete);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(searchEdit);
    mainLayout->addLayout(toolbarLayout);

    tableView = new QTableView(this);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setSortingEnabled(true);
    tableView->horizontalHeader()->setStretchLastSection(true);
    mainLayout->addWidget(tableView);

    connect(btnAdd, &QPushButton::clicked, this, &ClientsTab::onAddClicked);
    connect(btnEdit, &QPushButton::clicked, this, &ClientsTab::onEditClicked);
    connect(btnDelete, &QPushButton::clicked, this, &ClientsTab::onDeleteClicked);
    connect(searchEdit, &QLineEdit::textChanged, this, &ClientsTab::onSearchTextChanged);
}

void ClientsTab::setupModel() {
    model = new QSqlTableModel(this);
    model->setTable("clients");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();

    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Прізвище");
    model->setHeaderData(2, Qt::Horizontal, "Ім'я");
    model->setHeaderData(3, Qt::Horizontal, "По батькові");
    model->setHeaderData(4, Qt::Horizontal, "Телефон");
    model->setHeaderData(5, Qt::Horizontal, "Email");
    model->setHeaderData(6, Qt::Horizontal, "Адреса");
    model->setHeaderData(7, Qt::Horizontal, "Дата реєстрації");

    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterKeyColumn(-1); // -1 означає пошук по ВСІХ колонках

    tableView->setModel(proxyModel);
    tableView->hideColumn(0); // Ховаємо ID
}

void ClientsTab::onAddClicked() {
    QDialog dialog(this);
    dialog.setWindowTitle("Додати клієнта");
    QFormLayout form(&dialog);

    QLineEdit lnEdit, fnEdit, patrEdit, phoneEdit, emailEdit, addrEdit;
    
    // Валідатор для телефону: обов'язково починається з +380 і має 9 цифр після
    QRegularExpression phoneRx("^\\+380\\d{9}$");
    phoneEdit.setValidator(new QRegularExpressionValidator(phoneRx, &dialog));
    phoneEdit.setPlaceholderText("+380XXXXXXXXX");

    form.addRow("Прізвище *:", &lnEdit);
    form.addRow("Ім'я *:", &fnEdit);
    form.addRow("По батькові:", &patrEdit);
    form.addRow("Телефон *:", &phoneEdit);
    form.addRow("Email:", &emailEdit);
    form.addRow("Адреса:", &addrEdit);

    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form.addRow(&buttons);
    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        if (lnEdit.text().trimmed().isEmpty() || fnEdit.text().trimmed().isEmpty() || phoneEdit.text().trimmed().isEmpty()) {

            QSqlQuery checkQuery;
            checkQuery.prepare("SELECT client_id FROM clients WHERE phone = :phone");
            checkQuery.bindValue(":phone", phoneEdit.text().trimmed());
            if (checkQuery.exec() && checkQuery.next()) {
                QMessageBox::warning(this, "Помилка", "Клієнт із таким номером телефону вже існує!");
                return;
            }

            QMessageBox::warning(this, "Помилка", "Прізвище, Ім'я та Телефон є обов'язковими полями!");
            return;
        }

        int row = model->rowCount();
        model->insertRow(row);
        model->setData(model->index(row, 1), lnEdit.text().trimmed());
        model->setData(model->index(row, 2), fnEdit.text().trimmed());
        model->setData(model->index(row, 3), patrEdit.text().trimmed());
        model->setData(model->index(row, 4), phoneEdit.text().trimmed());
        model->setData(model->index(row, 5), emailEdit.text().trimmed());
        model->setData(model->index(row, 6), addrEdit.text().trimmed());
        // registration_date заповниться автоматично базою даних
        
        if (!model->submitAll()) {
            QMessageBox::critical(this, "Помилка", model->lastError().text());
            model->revertAll();
        } else {
            model->select();
        }
    }
}

void ClientsTab::onEditClicked() {
    QModelIndex proxyIndex = tableView->currentIndex();
    if (!proxyIndex.isValid()) return;

    int row = proxyModel->mapToSource(proxyIndex).row();
    
    QDialog dialog(this);
    dialog.setWindowTitle("Редагувати клієнта");
    QFormLayout form(&dialog);

    QLineEdit lnEdit(model->data(model->index(row, 1)).toString());
    QLineEdit fnEdit(model->data(model->index(row, 2)).toString());
    QLineEdit patrEdit(model->data(model->index(row, 3)).toString());
    QLineEdit phoneEdit(model->data(model->index(row, 4)).toString());
    QLineEdit emailEdit(model->data(model->index(row, 5)).toString());
    QLineEdit addrEdit(model->data(model->index(row, 6)).toString());

    QRegularExpression phoneRx("^\\+380\\d{9}$");
    phoneEdit.setValidator(new QRegularExpressionValidator(phoneRx, &dialog));

    form.addRow("Прізвище *:", &lnEdit);
    form.addRow("Ім'я *:", &fnEdit);
    form.addRow("По батькові:", &patrEdit);
    form.addRow("Телефон *:", &phoneEdit);
    form.addRow("Email:", &emailEdit);
    form.addRow("Адреса:", &addrEdit);

    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form.addRow(&buttons);
    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        if (lnEdit.text().trimmed().isEmpty() || fnEdit.text().trimmed().isEmpty() || phoneEdit.text().trimmed().isEmpty()) {

            int currentId = model->data(model->index(row, 0)).toInt();
            QSqlQuery checkQuery;
            checkQuery.prepare("SELECT client_id FROM clients WHERE phone = :phone AND client_id != :id");
            checkQuery.bindValue(":phone", phoneEdit.text().trimmed());
            checkQuery.bindValue(":id", currentId);
            if (checkQuery.exec() && checkQuery.next()) {
                QMessageBox::warning(this, "Помилка", "Інший клієнт вже використовує цей номер телефону!");
                return;
            }

            QMessageBox::warning(this, "Помилка", "Прізвище, Ім'я та Телефон є обов'язковими полями!");
            return;
        }

        model->setData(model->index(row, 1), lnEdit.text().trimmed());
        model->setData(model->index(row, 2), fnEdit.text().trimmed());
        model->setData(model->index(row, 3), patrEdit.text().trimmed());
        model->setData(model->index(row, 4), phoneEdit.text().trimmed());
        model->setData(model->index(row, 5), emailEdit.text().trimmed());
        model->setData(model->index(row, 6), addrEdit.text().trimmed());

        if (!model->submitAll()) {
            QMessageBox::critical(this, "Помилка", model->lastError().text());
            model->revertAll();
        }
    }
}

void ClientsTab::onDeleteClicked() {
    QModelIndex proxyIndex = tableView->currentIndex();
    if (!proxyIndex.isValid()) return;

    if (QMessageBox::question(this, "Підтвердження", "Ви впевнені, що хочете видалити клієнта?") == QMessageBox::Yes) {
        model->removeRow(proxyModel->mapToSource(proxyIndex).row());
        if (!model->submitAll()) {
            QMessageBox::critical(this, "Помилка видалення", "Неможливо видалити клієнта. У нього є пов'язані замовлення.");
            model->revertAll();
        }
    }
}

void ClientsTab::onSearchTextChanged(const QString &text) {
    proxyModel->setFilterRegularExpression(text);
}