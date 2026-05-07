#include "PositionsTab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlError>
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QShowEvent>

PositionsTab::PositionsTab(QWidget *parent) : QWidget(parent) {
    setupUi();
    setupModel();
}

void PositionsTab::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *toolbarLayout = new QHBoxLayout();
    
    QPushButton *btnAdd = new QPushButton("Додати", this);
    QPushButton *btnEdit = new QPushButton("Редагувати", this);
    QPushButton *btnDelete = new QPushButton("Видалити", this);
    QLineEdit *searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("Пошук посади...");

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

    connect(btnAdd, &QPushButton::clicked, this, &PositionsTab::onAddClicked);
    connect(btnEdit, &QPushButton::clicked, this, &PositionsTab::onEditClicked);
    connect(btnDelete, &QPushButton::clicked, this, &PositionsTab::onDeleteClicked);
    connect(searchEdit, &QLineEdit::textChanged, this, &PositionsTab::onSearchTextChanged);
}

void PositionsTab::setupModel() {
    model = new QSqlTableModel(this);
    model->setTable("positions");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();

    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Назва посади");
    model->setHeaderData(2, Qt::Horizontal, "Годинна ставка (грн)");

    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterKeyColumn(1); // Пошук по назві

    tableView->setModel(proxyModel);
    tableView->hideColumn(0);
}

void PositionsTab::onAddClicked() {
    QDialog dialog(this);
    dialog.setWindowTitle("Додати посаду");
    QFormLayout form(&dialog);

    QLineEdit nameEdit(&dialog);
    QDoubleSpinBox rateSpin(&dialog);
    rateSpin.setMaximum(10000.0);
    rateSpin.setDecimals(2);

    form.addRow("Назва посади:", &nameEdit);
    form.addRow("Ставка за годину:", &rateSpin);

    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form.addRow(&buttons);
    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        if (nameEdit.text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Увага", "Назва посади не може бути порожньою!");
            return;
        }

        int row = model->rowCount();
        model->insertRow(row);
        model->setData(model->index(row, 1), nameEdit.text().trimmed());
        model->setData(model->index(row, 2), rateSpin.value());
        
        if (!model->submitAll()) {
            QMessageBox::critical(this, "Помилка", model->lastError().text());
            model->revertAll();
        }
    }
}

void PositionsTab::onEditClicked() {
    QModelIndex proxyIndex = tableView->currentIndex();
    if (!proxyIndex.isValid()) {
        QMessageBox::warning(this, "Увага", "Виберіть посаду для редагування.");
        return;
    }

    int row = proxyModel->mapToSource(proxyIndex).row();
    QString currentName = model->data(model->index(row, 1)).toString();
    double currentRate = model->data(model->index(row, 2)).toDouble();

    QDialog dialog(this);
    dialog.setWindowTitle("Редагувати посаду");
    QFormLayout form(&dialog);

    QLineEdit nameEdit(currentName, &dialog);
    QDoubleSpinBox rateSpin(&dialog);
    rateSpin.setMaximum(10000.0);
    rateSpin.setValue(currentRate);

    form.addRow("Назва посади:", &nameEdit);
    form.addRow("Ставка за годину:", &rateSpin);

    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form.addRow(&buttons);
    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        if (nameEdit.text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Увага", "Назва посади не може бути порожньою!");
            return;
        }

        model->setData(model->index(row, 1), nameEdit.text().trimmed());
        model->setData(model->index(row, 2), rateSpin.value());
        
        if (!model->submitAll()) {
            QMessageBox::critical(this, "Помилка", model->lastError().text());
            model->revertAll();
        }
    }
}

void PositionsTab::onDeleteClicked() {
    QModelIndex proxyIndex = tableView->currentIndex();
    if (!proxyIndex.isValid()) return;

    if (QMessageBox::question(this, "Підтвердження", "Видалити посаду?") == QMessageBox::Yes) {
        model->removeRow(proxyModel->mapToSource(proxyIndex).row());
        if (!model->submitAll()) {
            QMessageBox::critical(this, "Помилка", "Неможливо видалити (можливо є працівники на цій посаді).");
            model->revertAll();
        }
    }
}

void PositionsTab::onSearchTextChanged(const QString &text) {
    proxyModel->setFilterRegularExpression(text);
}

void PositionsTab::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    model->select(); // Оновлює дані з бази
}