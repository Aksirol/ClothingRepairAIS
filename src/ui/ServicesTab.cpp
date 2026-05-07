#include "ServicesTab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlError>
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QSqlQuery>
#include <QSqlRelation>
#include <QShowEvent>

ServicesTab::ServicesTab(QWidget *parent) : QWidget(parent) {
    setupUi();
    setupModel();
}

void ServicesTab::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *toolbarLayout = new QHBoxLayout();

    QPushButton *btnAdd = new QPushButton("Додати", this);
    QPushButton *btnEdit = new QPushButton("Редагувати", this);
    QPushButton *btnDelete = new QPushButton("Видалити", this);
    QLineEdit *searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("Пошук послуги...");

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

    connect(btnAdd, &QPushButton::clicked, this, &ServicesTab::onAddClicked);
    connect(btnEdit, &QPushButton::clicked, this, &ServicesTab::onEditClicked);
    connect(btnDelete, &QPushButton::clicked, this, &ServicesTab::onDeleteClicked);
    connect(searchEdit, &QLineEdit::textChanged, this, &ServicesTab::onSearchTextChanged);
}

void ServicesTab::setupModel() {
    model = new QSqlRelationalTableModel(this);
    model->setTable("repair_services");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->setRelation(1, QSqlRelation("repair_categories", "category_id", "category_name"));
    model->select();

    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Категорія");
    model->setHeaderData(2, Qt::Horizontal, "Назва послуги");
    model->setHeaderData(3, Qt::Horizontal, "Опис");
    model->setHeaderData(4, Qt::Horizontal, "Базова ціна");
    model->setHeaderData(5, Qt::Horizontal, "Днів на виконання");

    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterKeyColumn(-1);

    tableView->setModel(proxyModel);
    tableView->hideColumn(0);
}

void ServicesTab::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    model->select();
}

void ServicesTab::populateCategoriesCombo(QComboBox *combo) {
    QSqlQuery q("SELECT category_id, category_name FROM repair_categories");
    while(q.next()) {
        combo->addItem(q.value(1).toString(), q.value(0).toInt());
    }
}

void ServicesTab::onAddClicked() {
    QDialog dialog(this);
    dialog.setWindowTitle("Додати послугу");
    QFormLayout form(&dialog);

    QComboBox catCombo(&dialog);
    populateCategoriesCombo(&catCombo);
    QLineEdit nameEdit, descEdit;
    QDoubleSpinBox priceSpin; priceSpin.setMaximum(100000);
    QSpinBox daysSpin; daysSpin.setMaximum(365);

    form.addRow("Категорія *:", &catCombo);
    form.addRow("Назва *:", &nameEdit);
    form.addRow("Опис:", &descEdit);
    form.addRow("Ціна *:", &priceSpin);
    form.addRow("Термін (днів) *:", &daysSpin);

    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form.addRow(&buttons);
    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        if (nameEdit.text().trimmed().isEmpty()) return;
        int row = model->rowCount();
        model->insertRow(row);
        model->setData(model->index(row, 1), catCombo.currentData());
        model->setData(model->index(row, 2), nameEdit.text().trimmed());
        model->setData(model->index(row, 3), descEdit.text().trimmed());
        model->setData(model->index(row, 4), priceSpin.value());
        model->setData(model->index(row, 5), daysSpin.value());
        if (!model->submitAll()) model->revertAll();
    }
}

void ServicesTab::onEditClicked() {
    QModelIndex proxyIndex = tableView->currentIndex();
    if (!proxyIndex.isValid()) return;
    int row = proxyModel->mapToSource(proxyIndex).row();

    QDialog dialog(this);
    dialog.setWindowTitle("Редагувати послугу");
    QFormLayout form(&dialog);

    QComboBox catCombo(&dialog);
    populateCategoriesCombo(&catCombo);
    int catIdx = catCombo.findText(model->data(model->index(row, 1)).toString());
    if (catIdx >= 0) catCombo.setCurrentIndex(catIdx);

    QLineEdit nameEdit(model->data(model->index(row, 2)).toString());
    QLineEdit descEdit(model->data(model->index(row, 3)).toString());
    QDoubleSpinBox priceSpin; priceSpin.setMaximum(100000); priceSpin.setValue(model->data(model->index(row, 4)).toDouble());
    QSpinBox daysSpin; daysSpin.setMaximum(365); daysSpin.setValue(model->data(model->index(row, 5)).toInt());

    form.addRow("Категорія *:", &catCombo);
    form.addRow("Назва *:", &nameEdit);
    form.addRow("Опис:", &descEdit);
    form.addRow("Ціна *:", &priceSpin);
    form.addRow("Термін (днів) *:", &daysSpin);

    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form.addRow(&buttons);
    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        model->setData(model->index(row, 1), catCombo.currentData());
        model->setData(model->index(row, 2), nameEdit.text().trimmed());
        model->setData(model->index(row, 3), descEdit.text().trimmed());
        model->setData(model->index(row, 4), priceSpin.value());
        model->setData(model->index(row, 5), daysSpin.value());
        if (!model->submitAll()) model->revertAll();
    }
}

void ServicesTab::onDeleteClicked() {
    QModelIndex proxyIndex = tableView->currentIndex();
    if (!proxyIndex.isValid()) return;
    if (QMessageBox::question(this, "Видалення", "Видалити послугу?") == QMessageBox::Yes) {
        model->removeRow(proxyModel->mapToSource(proxyIndex).row());
        if (!model->submitAll()) {
            QMessageBox::critical(this, "Помилка", "Послуга вже використовується в замовленнях.");
            model->revertAll();
        }
    }
}

void ServicesTab::onSearchTextChanged(const QString &text) {
    proxyModel->setFilterRegularExpression(text);
}