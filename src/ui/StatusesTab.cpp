#include "StatusesTab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlError>
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>

StatusesTab::StatusesTab(QWidget *parent) : QWidget(parent) {
    setupUi();
    setupModel();
}

void StatusesTab::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Панель інструментів
    QHBoxLayout *toolbarLayout = new QHBoxLayout();
    
    QPushButton *btnAdd = new QPushButton("Додати", this);
    QPushButton *btnEdit = new QPushButton("Редагувати", this);
    QPushButton *btnDelete = new QPushButton("Видалити", this);
    QLineEdit *searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("Пошук статусу...");

    toolbarLayout->addWidget(btnAdd);
    toolbarLayout->addWidget(btnEdit);
    toolbarLayout->addWidget(btnDelete);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(searchEdit);

    mainLayout->addLayout(toolbarLayout);

    // Таблиця
    tableView = new QTableView(this);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setSortingEnabled(true);
    tableView->horizontalHeader()->setStretchLastSection(true);

    mainLayout->addWidget(tableView);

    // Підключення сигналів
    connect(btnAdd, &QPushButton::clicked, this, &StatusesTab::onAddClicked);
    connect(btnEdit, &QPushButton::clicked, this, &StatusesTab::onEditClicked);
    connect(btnDelete, &QPushButton::clicked, this, &StatusesTab::onDeleteClicked);
    connect(searchEdit, &QLineEdit::textChanged, this, &StatusesTab::onSearchTextChanged);
}

void StatusesTab::setupModel() {
    model = new QSqlTableModel(this);
    model->setTable("order_statuses");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();

    // Налаштовуємо заголовки колонок
    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Назва статусу");
    model->setHeaderData(2, Qt::Horizontal, "HEX Колір");

    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterKeyColumn(1); // Шукаємо лише по назві статусу

    tableView->setModel(proxyModel);
    tableView->hideColumn(0); // Ховаємо ID
}

void StatusesTab::onAddClicked() {
    QDialog dialog(this);
    dialog.setWindowTitle("Додати статус");
    QFormLayout form(&dialog);

    QLineEdit nameEdit(&dialog);
    QLineEdit colorEdit(&dialog);
    colorEdit.setPlaceholderText("Наприклад: #FF0000"); // Підказка формату

    form.addRow("Назва статусу:", &nameEdit);
    form.addRow("Код кольору:", &colorEdit);

    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form.addRow(&buttons);
    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        if (nameEdit.text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Увага", "Назва статусу не може бути порожньою!");
            return;
        }

        int row = model->rowCount();
        model->insertRow(row);
        model->setData(model->index(row, 1), nameEdit.text().trimmed());
        model->setData(model->index(row, 2), colorEdit.text().trimmed());
        
        if (!model->submitAll()) {
            QMessageBox::critical(this, "Помилка", "Не вдалося зберегти:\n" + model->lastError().text());
            model->revertAll();
        } else {
            model->select(); // Оновлюємо дані для впевненості
        }
    }
}

void StatusesTab::onEditClicked() {
    QModelIndex proxyIndex = tableView->currentIndex();
    if (!proxyIndex.isValid()) {
        QMessageBox::warning(this, "Увага", "Виберіть статус для редагування.");
        return;
    }

    int row = proxyModel->mapToSource(proxyIndex).row();
    QString currentName = model->data(model->index(row, 1)).toString();
    QString currentColor = model->data(model->index(row, 2)).toString();

    QDialog dialog(this);
    dialog.setWindowTitle("Редагувати статус");
    QFormLayout form(&dialog);

    QLineEdit nameEdit(currentName, &dialog);
    QLineEdit colorEdit(currentColor, &dialog);
    colorEdit.setPlaceholderText("Наприклад: #FF0000");

    form.addRow("Назва статусу:", &nameEdit);
    form.addRow("Код кольору:", &colorEdit);

    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form.addRow(&buttons);
    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        if (nameEdit.text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Увага", "Назва статусу не може бути порожньою!");
            return;
        }

        // Перевіряємо, чи були зміни
        if (nameEdit.text().trimmed() != currentName || colorEdit.text().trimmed() != currentColor) {
            model->setData(model->index(row, 1), nameEdit.text().trimmed());
            model->setData(model->index(row, 2), colorEdit.text().trimmed());
            
            if (!model->submitAll()) {
                QMessageBox::critical(this, "Помилка", "Не вдалося оновити:\n" + model->lastError().text());
                model->revertAll();
            }
        }
    }
}

void StatusesTab::onDeleteClicked() {
    QModelIndex proxyIndex = tableView->currentIndex();
    if (!proxyIndex.isValid()) {
        QMessageBox::warning(this, "Увага", "Виберіть статус для видалення.");
        return;
    }

    // Додаткова перевірка: заборонимо видаляти системні статуси (ID 1-5)
    int row = proxyModel->mapToSource(proxyIndex).row();
    int statusId = model->data(model->index(row, 0)).toInt();
    
    if (statusId >= 1 && statusId <= 5) {
        QMessageBox::warning(this, "Заборонено", "Видалення базових системних статусів (Прийнято, В роботі, Готово, Видано, Скасовано) заборонено бізнес-логікою.");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Підтвердження", 
                                  "Ви впевнені, що хочете видалити цей статус?",
                                  QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        model->removeRow(row);
        
        if (!model->submitAll()) {
            QMessageBox::critical(this, "Помилка", "Неможливо видалити статус. Ймовірно, існують замовлення з цим статусом.");
            model->revertAll();
        } else {
            model->select();
        }
    }
}

void StatusesTab::onSearchTextChanged(const QString &text) {
    proxyModel->setFilterRegularExpression(text);
}