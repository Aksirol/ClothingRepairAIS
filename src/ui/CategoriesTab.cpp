#include "CategoriesTab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlError>
#include <QInputDialog> // Використовуємо просте діалогове вікно замість створення окремого класу

CategoriesTab::CategoriesTab(QWidget *parent) : QWidget(parent) {
    setupUi();
    setupModel();
}

void CategoriesTab::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Панель інструментів (кнопки та пошук)
    QHBoxLayout *toolbarLayout = new QHBoxLayout();
    
    btnAdd = new QPushButton("Додати", this);
    btnEdit = new QPushButton("Редагувати", this);
    btnDelete = new QPushButton("Видалити", this);
    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("Пошук категорії...");

    toolbarLayout->addWidget(btnAdd);
    toolbarLayout->addWidget(btnEdit);
    toolbarLayout->addWidget(btnDelete);
    toolbarLayout->addStretch(); // Відштовхує пошук праворуч
    toolbarLayout->addWidget(searchEdit);

    mainLayout->addLayout(toolbarLayout);

    // Таблиця
    tableView = new QTableView(this);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows); // Виділяємо цілі рядки
    tableView->setSelectionMode(QAbstractItemView::SingleSelection); // Лише один рядок за раз
    tableView->setSortingEnabled(true); // Дозволяємо сортування по кліку на заголовок
    tableView->horizontalHeader()->setStretchLastSection(true); // Розтягуємо останню колонку

    mainLayout->addWidget(tableView);

    // Підключення сигналів
    connect(btnAdd, &QPushButton::clicked, this, &CategoriesTab::onAddClicked);
    connect(btnEdit, &QPushButton::clicked, this, &CategoriesTab::onEditClicked);
    connect(btnDelete, &QPushButton::clicked, this, &CategoriesTab::onDeleteClicked);
    connect(searchEdit, &QLineEdit::textChanged, this, &CategoriesTab::onSearchTextChanged);
}

void CategoriesTab::setupModel() {
    // Підключаємося до таблиці БД
    model = new QSqlTableModel(this);
    model->setTable("repair_categories");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit); // Зміни зберігаємо вручну
    model->select(); // Завантажуємо дані з БД

    // Змінюємо назви колонок на зрозумілі
    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Назва категорії");

    // Налаштовуємо проксі-модель для пошуку та сортування
    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive); // Пошук незалежний від регістру
    proxyModel->setFilterKeyColumn(1); // Шукаємо лише в колонці "Назва категорії"

    tableView->setModel(proxyModel);
    tableView->hideColumn(0); // Ховаємо колонку ID, користувачу вона не потрібна
}

void CategoriesTab::onAddClicked() {
    bool ok;
    QString newName = QInputDialog::getText(this, "Додати категорію",
                                            "Назва нової категорії:", QLineEdit::Normal,
                                            "", &ok);
    
    // Якщо користувач натиснув "ОК"
    if (ok) {
        // Перевіряємо, чи поле не порожнє
        if (newName.trimmed().isEmpty()) {
            QMessageBox::warning(this, "Увага", "Назва категорії не може бути порожньою!");
            return; // Зупиняємо виконання
        }

        int row = model->rowCount();
        model->insertRow(row);
        model->setData(model->index(row, 1), newName.trimmed());

        if (!model->submitAll()) {
            QMessageBox::critical(this, "Помилка", "Не вдалося зберегти:\n" + model->lastError().text());
            model->revertAll();
        } else {
            model->select(); // Оновлюємо дані
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

    // Якщо користувач натиснув "ОК"
    if (ok) {
        // Перевіряємо на порожнечу
        if (newName.trimmed().isEmpty()) {
            QMessageBox::warning(this, "Увага", "Назва категорії не може бути порожньою!");
            return;
        }

        // Зберігаємо, лише якщо назва дійсно змінилася
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

    // Підтвердження видалення
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Підтвердження", "Ви впевнені, що хочете видалити цю категорію?",
                                  QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        QModelIndex sourceIndex = proxyModel->mapToSource(proxyIndex);
        model->removeRow(sourceIndex.row());
        
        if (!model->submitAll()) {
            // Тут спрацює наш FOREIGN KEY захист, якщо до категорії прив'язані послуги
            QMessageBox::critical(this, "Помилка", "Неможливо видалити категорію. Можливо, до неї вже прив'язані послуги.");
            model->revertAll();
        } else {
            model->select();
        }
    }
}

void CategoriesTab::onSearchTextChanged(const QString &text) {
    // Встановлюємо фільтр для проксі-моделі на основі введеного тексту
    proxyModel->setFilterRegularExpression(text);
}