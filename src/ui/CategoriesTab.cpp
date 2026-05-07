#include "CategoriesTab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlError>
#include <QInputDialog>
#include <QPushButton> // Перенесли з .h
#include <QLineEdit>   // Перенесли з .h
#include <QShowEvent>  // Для showEvent

CategoriesTab::CategoriesTab(QWidget *parent) : QWidget(parent) {
    setupUi();
    setupModel();
}

void CategoriesTab::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Панель інструментів (кнопки та пошук)
    QHBoxLayout *toolbarLayout = new QHBoxLayout();

    // ОГОЛОШУЄМО ЛОКАЛЬНО: додано типи QPushButton* та QLineEdit*
    QPushButton *btnAdd = new QPushButton("Додати", this);
    QPushButton *btnEdit = new QPushButton("Редагувати", this);
    QPushButton *btnDelete = new QPushButton("Видалити", this);
    QLineEdit *searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("Пошук категорії...");

    toolbarLayout->addWidget(btnAdd);
    toolbarLayout->addWidget(btnEdit);
    toolbarLayout->addWidget(btnDelete);
    toolbarLayout->addStretch(); // Відштовхує пошук праворуч
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
    connect(btnAdd, &QPushButton::clicked, this, &CategoriesTab::onAddClicked);
    connect(btnEdit, &QPushButton::clicked, this, &CategoriesTab::onEditClicked);
    connect(btnDelete, &QPushButton::clicked, this, &CategoriesTab::onDeleteClicked);
    connect(searchEdit, &QLineEdit::textChanged, this, &CategoriesTab::onSearchTextChanged);
}

void CategoriesTab::setupModel() {
    model = new QSqlTableModel(this);
    model->setTable("repair_categories");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();

    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Назва категорії");

    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterKeyColumn(1);

    tableView->setModel(proxyModel);
    tableView->hideColumn(0);
}

void CategoriesTab::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    if (model) {
        model->select(); // Автоматичне оновлення даних при перемиканні на вкладку
    }
}

void CategoriesTab::onAddClicked() {
    bool ok;
    QString newName = QInputDialog::getText(this, "Додати категорію",
                                            "Назва нової категорії:", QLineEdit::Normal,
                                            "", &ok);

    if (ok) {
        if (newName.trimmed().isEmpty()) {
            QMessageBox::warning(this, "Увага", "Назва категорії не може бути порожньою!");
            return;
        }

        int row = model->rowCount();
        model->insertRow(row);
        model->setData(model->index(row, 1), newName.trimmed());

        if (!model->submitAll()) {
            QMessageBox::critical(this, "Помилка", "Не вдалося зберегти:\n" + model->lastError().text());
            model->revertAll();
        } else {
            model->select();
        }
    }
}

void CategoriesTab::onEditClicked() {
    QModelIndex proxyIndex = tableView->currentIndex();
    if (!proxyIndex.isValid()) {
        QMessageBox::warning(this, "Увага", "Виберіть категорію для редагування.");
        return;
    }

    QModelIndex sourceIndex = proxyModel->mapToSource(proxyIndex);
    int row = sourceIndex.row();
    QString currentName = model->data(model->index(row, 1)).toString();

    bool ok;
    QString newName = QInputDialog::getText(this, "Редагувати",
                                            "Нова назва:", QLineEdit::Normal,
                                            currentName, &ok);

    if (ok) {
        if (newName.trimmed().isEmpty()) {
            QMessageBox::warning(this, "Увага", "Назва категорії не може бути порожньою!");
            return;
        }

        if (newName != currentName) {
            model->setData(model->index(row, 1), newName.trimmed());
            if (!model->submitAll()) {
                QMessageBox::critical(this, "Помилка", "Не вдалося оновити:\n" + model->lastError().text());
                model->revertAll();
            }
        }
    }
}

void CategoriesTab::onDeleteClicked() {
    QModelIndex proxyIndex = tableView->currentIndex();
    if (!proxyIndex.isValid()) {
        QMessageBox::warning(this, "Увага", "Виберіть категорію для видалення.");
        return;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Підтвердження", "Ви впевнені, що хочете видалити цю категорію?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QModelIndex sourceIndex = proxyModel->mapToSource(proxyIndex);
        model->removeRow(sourceIndex.row());

        if (!model->submitAll()) {
            QMessageBox::critical(this, "Помилка", "Неможливо видалити категорію. Можливо, до неї вже прив'язані послуги.");
            model->revertAll();
        } else {
            model->select();
        }
    }
}

void CategoriesTab::onSearchTextChanged(const QString &text) {
    proxyModel->setFilterRegularExpression(text);
}