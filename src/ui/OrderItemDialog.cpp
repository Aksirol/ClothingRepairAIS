#include "OrderItemDialog.h"
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QSqlQuery>
#include <QMessageBox>

OrderItemDialog::OrderItemDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Додати послугу до замовлення");
    setupUi();
    loadCategories();
}

void OrderItemDialog::setupUi() {
    QFormLayout *form = new QFormLayout(this);

    categoryCombo = new QComboBox(this);
    serviceCombo = new QComboBox(this);
    
    quantitySpin = new QSpinBox(this);
    quantitySpin->setMinimum(1);
    quantitySpin->setValue(1);

    priceSpin = new QDoubleSpinBox(this);
    priceSpin->setMaximum(100000.0);
    priceSpin->setDecimals(2);

    descriptionEdit = new QLineEdit(this);
    descriptionEdit->setPlaceholderText("Наприклад: Чорна шкіряна куртка");

    form->addRow("Категорія:", categoryCombo);
    form->addRow("Послуга *:", serviceCombo);
    form->addRow("Опис речі:", descriptionEdit);
    form->addRow("Кількість *:", quantitySpin);
    form->addRow("Ціна за од. *:", priceSpin);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    form->addRow(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    
    connect(categoryCombo, &QComboBox::currentIndexChanged, this, &OrderItemDialog::onCategoryChanged);
    connect(serviceCombo, &QComboBox::currentIndexChanged, this, &OrderItemDialog::onServiceChanged);
}

void OrderItemDialog::loadCategories() {
    categoryCombo->clear();
    QSqlQuery q("SELECT category_id, category_name FROM repair_categories ORDER BY category_name");
    while (q.next()) {
        categoryCombo->addItem(q.value(1).toString(), q.value(0).toInt());
    }
    categoryCombo->setCurrentIndex(-1); // Скидаємо вибір
}

void OrderItemDialog::onCategoryChanged() {
    serviceCombo->clear();
    priceSpin->setValue(0.0);
    
    int catId = categoryCombo->currentData().toInt();
    if (catId <= 0) return;

    QSqlQuery q;
    q.prepare("SELECT service_id, service_name, base_price FROM repair_services WHERE category_id = :id ORDER BY service_name");
    q.bindValue(":id", catId);
    if (q.exec()) {
        while (q.next()) {
            // Зберігаємо ID послуги та її ціну
            serviceCombo->addItem(q.value(1).toString(), q.value(0).toInt());
        }
    }
    serviceCombo->setCurrentIndex(-1);
}

void OrderItemDialog::onServiceChanged() {
    int serviceId = serviceCombo->currentData().toInt();
    if (serviceId <= 0) return;

    // Підтягуємо ціну при виборі послуги
    QSqlQuery q;
    q.prepare("SELECT base_price FROM repair_services WHERE service_id = :id");
    q.bindValue(":id", serviceId);
    if (q.exec() && q.next()) {
        priceSpin->setValue(q.value(0).toDouble()); // Користувач може змінити її після цього
    }
}

int OrderItemDialog::getServiceId() const { return serviceCombo->currentData().toInt(); }
QString OrderItemDialog::getServiceName() const { return serviceCombo->currentText(); }
int OrderItemDialog::getQuantity() const { return quantitySpin->value(); }
double OrderItemDialog::getUnitPrice() const { return priceSpin->value(); }
QString OrderItemDialog::getDescription() const { return descriptionEdit->text().trimmed(); }