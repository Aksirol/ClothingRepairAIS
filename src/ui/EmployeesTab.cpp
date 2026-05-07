#include "EmployeesTab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlError>
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QShowEvent>
#include <QComboBox>
#include <QDateEdit>
#include <QSqlQuery>
#include <QSqlRelation>

EmployeesTab::EmployeesTab(QWidget *parent) : QWidget(parent) {
    setupUi();
    setupModel();
}

void EmployeesTab::setupUi() {
    // Структура UI ідентична ClientsTab
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *toolbarLayout = new QHBoxLayout();
    
    QPushButton *btnAdd = new QPushButton("Додати", this);
    QPushButton *btnEdit = new QPushButton("Редагувати", this);
    QPushButton *btnDelete = new QPushButton("Видалити", this);
    QLineEdit *searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("Пошук співробітника...");

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

    connect(btnAdd, &QPushButton::clicked, this, &EmployeesTab::onAddClicked);
    connect(btnEdit, &QPushButton::clicked, this, &EmployeesTab::onEditClicked);
    connect(btnDelete, &QPushButton::clicked, this, &EmployeesTab::onDeleteClicked);
    connect(searchEdit, &QLineEdit::textChanged, this, &EmployeesTab::onSearchTextChanged);
}

void EmployeesTab::setupModel() {
    model = new QSqlRelationalTableModel(this);
    model->setTable("employees");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);

    // Магія зв'язків: колонка 1 (position_id) мапиться на таблицю positions
    model->setRelation(1, QSqlRelation("positions", "position_id", "position_name"));
    model->select();

    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Посада");
    model->setHeaderData(2, Qt::Horizontal, "Прізвище");
    model->setHeaderData(3, Qt::Horizontal, "Ім'я");
    model->setHeaderData(4, Qt::Horizontal, "По батькові");
    model->setHeaderData(5, Qt::Horizontal, "Телефон");
    model->setHeaderData(6, Qt::Horizontal, "Дата найму");

    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterKeyColumn(-1);

    tableView->setModel(proxyModel);
    tableView->hideColumn(0);
}

// Допоміжний метод для завантаження посад у ComboBox
void EmployeesTab::populatePositionsCombo(QComboBox *combo) {
    QSqlQuery q("SELECT position_id, position_name FROM positions");
    while(q.next()) {
        // Додаємо текст, і приховано зберігаємо ID
        combo->addItem(q.value(1).toString(), q.value(0).toInt());
    }
}

void EmployeesTab::onAddClicked() {
    QDialog dialog(this);
    dialog.setWindowTitle("Додати співробітника");
    QFormLayout form(&dialog);

    QComboBox posCombo(&dialog);
    populatePositionsCombo(&posCombo);

    QLineEdit lnEdit, fnEdit, patrEdit, phoneEdit;
    QDateEdit hireDateEdit(QDate::currentDate(), &dialog);
    hireDateEdit.setCalendarPopup(true);

    form.addRow("Посада *:", &posCombo);
    form.addRow("Прізвище *:", &lnEdit);
    form.addRow("Ім'я *:", &fnEdit);
    form.addRow("По батькові:", &patrEdit);
    form.addRow("Телефон *:", &phoneEdit);
    form.addRow("Дата найму:", &hireDateEdit);

    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form.addRow(&buttons);
    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        if (lnEdit.text().trimmed().isEmpty() || fnEdit.text().trimmed().isEmpty() || phoneEdit.text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Помилка", "Заповніть усі обов'язкові поля!");
            return;
        }

        int row = model->rowCount();
        model->insertRow(row);
        
        // Отримуємо збережений ID посади
        int positionId = posCombo.currentData().toInt();
        
        // Для QSqlRelationalTableModel ми записуємо саме ID у колонку зв'язку
        model->setData(model->index(row, 1), positionId); 
        model->setData(model->index(row, 2), lnEdit.text().trimmed());
        model->setData(model->index(row, 3), fnEdit.text().trimmed());
        model->setData(model->index(row, 4), patrEdit.text().trimmed());
        model->setData(model->index(row, 5), phoneEdit.text().trimmed());
        model->setData(model->index(row, 6), hireDateEdit.date().toString(Qt::ISODate));
        
        if (!model->submitAll()) {
            QMessageBox::critical(this, "Помилка", model->lastError().text());
            model->revertAll();
        } else {
            model->select();
        }
    }
}

void EmployeesTab::onEditClicked() {
    QModelIndex proxyIndex = tableView->currentIndex();
    if (!proxyIndex.isValid()) return;

    int row = proxyModel->mapToSource(proxyIndex).row();
    
    QDialog dialog(this);
    dialog.setWindowTitle("Редагувати співробітника");
    QFormLayout form(&dialog);

    QComboBox posCombo(&dialog);
    populatePositionsCombo(&posCombo);
    // Шукаємо поточну посаду в ComboBox (порівнюємо текст, оскільки модель повертає текстове представлення зв'язку)
    QString currentPosName = model->data(model->index(row, 1)).toString();
    int comboIndex = posCombo.findText(currentPosName);
    if (comboIndex >= 0) posCombo.setCurrentIndex(comboIndex);

    QLineEdit lnEdit(model->data(model->index(row, 2)).toString(), &dialog);
    QLineEdit fnEdit(model->data(model->index(row, 3)).toString(), &dialog);
    QLineEdit patrEdit(model->data(model->index(row, 4)).toString(), &dialog);
    QLineEdit phoneEdit(model->data(model->index(row, 5)).toString(), &dialog);
    
    QDate currentDate = QDate::fromString(model->data(model->index(row, 6)).toString(), Qt::ISODate);
    QDateEdit hireDateEdit(currentDate, &dialog);
    hireDateEdit.setCalendarPopup(true);

    form.addRow("Посада *:", &posCombo);
    form.addRow("Прізвище *:", &lnEdit);
    form.addRow("Ім'я *:", &fnEdit);
    form.addRow("По батькові:", &patrEdit);
    form.addRow("Телефон *:", &phoneEdit);
    form.addRow("Дата найму:", &hireDateEdit);

    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form.addRow(&buttons);
    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        if (lnEdit.text().trimmed().isEmpty() || fnEdit.text().trimmed().isEmpty() || phoneEdit.text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Помилка", "Заповніть усі обов'язкові поля!");
            return;
        }

        model->setData(model->index(row, 1), posCombo.currentData().toInt());
        model->setData(model->index(row, 2), lnEdit.text().trimmed());
        model->setData(model->index(row, 3), fnEdit.text().trimmed());
        model->setData(model->index(row, 4), patrEdit.text().trimmed());
        model->setData(model->index(row, 5), phoneEdit.text().trimmed());
        model->setData(model->index(row, 6), hireDateEdit.date().toString(Qt::ISODate));

        if (!model->submitAll()) {
            QMessageBox::critical(this, "Помилка", model->lastError().text());
            model->revertAll();
        } else {
            model->select();
        }
    }
}

void EmployeesTab::onDeleteClicked() {
    QModelIndex proxyIndex = tableView->currentIndex();
    if (!proxyIndex.isValid()) return;

    if (QMessageBox::question(this, "Підтвердження", "Видалити співробітника?") == QMessageBox::Yes) {
        model->removeRow(proxyModel->mapToSource(proxyIndex).row());
        if (!model->submitAll()) {
            QMessageBox::critical(this, "Помилка видалення", "Співробітник вже має прив'язані замовлення.");
            model->revertAll();
        }
    }
}

void EmployeesTab::onSearchTextChanged(const QString &text) {
    proxyModel->setFilterRegularExpression(text);
}

void EmployeesTab::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    if (model) {
        model->select(); // Автоматично підтягуємо найсвіжіші дані з БД при відкритті вкладки
    }
}