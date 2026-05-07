#include "ReportsTab.h"
#include "ReportBuilder.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QTableView>
#include <QPushButton>
#include <QHeaderView>
#include <QGroupBox>
#include <QFormLayout>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

ReportsTab::ReportsTab(QWidget *parent) : QWidget(parent) {
    setupUi();
}

void ReportsTab::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QTabWidget *tabWidget = new QTabWidget(this);

    // ==========================================
    // ЗВІТ 1: Активні замовлення
    // ==========================================
    QWidget *activeTab = new QWidget();
    QVBoxLayout *activeLayout = new QVBoxLayout(activeTab);
    
    QPushButton *btnExportActive = new QPushButton("💾 Експорт у CSV", activeTab);
    activeLayout->addWidget(btnExportActive, 0, Qt::AlignRight);
    
    QTableView *activeTable = new QTableView(activeTab);
    activeModel = new QStandardItemModel(this);
    activeModel->setHorizontalHeaderLabels({"№ Замовлення", "Клієнт", "Майстер", "Статус", "Орієнтовна видача"});
    activeTable->setModel(activeModel);
    activeTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    activeTable->horizontalHeader()->setStretchLastSection(true);
    activeLayout->addWidget(activeTable);
    
    tabWidget->addTab(activeTab, "Активні замовлення");

    // ==========================================
    // ЗВІТ 2: Прострочені замовлення
    // ==========================================
    QWidget *overdueTab = new QWidget();
    QVBoxLayout *overdueLayout = new QVBoxLayout(overdueTab);
    
    QPushButton *btnExportOverdue = new QPushButton("💾 Експорт у CSV", overdueTab);
    overdueLayout->addWidget(btnExportOverdue, 0, Qt::AlignRight);
    
    QTableView *overdueTable = new QTableView(overdueTab);
    overdueModel = new QStandardItemModel(this);
    overdueModel->setHorizontalHeaderLabels({"№ Замовлення", "Клієнт", "Дата дедлайну", "Статус"});
    overdueTable->setModel(overdueModel);
    overdueTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    overdueTable->horizontalHeader()->setStretchLastSection(true);
    overdueTable->setStyleSheet("QTableView { background-color: #ffe6e6; }"); // Легкий червоний фон
    overdueLayout->addWidget(overdueTable);
    
    tabWidget->addTab(overdueTab, "⚠️ Прострочені");

    // ==========================================
    // ЗВІТ 3: Завантаженість майстрів
    // ==========================================
    QWidget *workloadTab = new QWidget();
    QVBoxLayout *workloadLayout = new QVBoxLayout(workloadTab);
    
    QPushButton *btnExportWorkload = new QPushButton("💾 Експорт у CSV", workloadTab);
    workloadLayout->addWidget(btnExportWorkload, 0, Qt::AlignRight);
    
    QTableView *workloadTable = new QTableView(workloadTab);
    workloadModel = new QStandardItemModel(this);
    workloadModel->setHorizontalHeaderLabels({"ПІБ Майстра", "Кількість активних замовлень"});
    workloadTable->setModel(workloadModel);
    workloadTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    workloadTable->horizontalHeader()->setStretchLastSection(true);
    workloadLayout->addWidget(workloadTable);
    
    tabWidget->addTab(workloadTab, "Завантаженість майстрів");

    // ==========================================
    // ЗВІТ 4: Фінансовий виторг (з фільтром дат)
    // ==========================================
    QWidget *revenueTab = new QWidget();
    QVBoxLayout *revenueLayout = new QVBoxLayout(revenueTab);
    
    QGroupBox *filterGroup = new QGroupBox("Параметри періоду", revenueTab);
    QFormLayout *formLayout = new QFormLayout(filterGroup);
    
    startDateEdit = new QDateEdit(QDate::currentDate().addMonths(-1), revenueTab); // За замовчуванням за місяць
    startDateEdit->setCalendarPopup(true);
    endDateEdit = new QDateEdit(QDate::currentDate(), revenueTab);
    endDateEdit->setCalendarPopup(true);
    
    formLayout->addRow("Початкова дата:", startDateEdit);
    formLayout->addRow("Кінцева дата:", endDateEdit);
    
    QPushButton *btnCalcRevenue = new QPushButton("Розрахувати виторг", revenueTab);
    formLayout->addRow("", btnCalcRevenue);
    
    revenueLayout->addWidget(filterGroup);
    
    lblRevenueResult = new QLabel("Виторг за обраний період: <b>0.00 грн</b>", revenueTab);
    lblRevenueResult->setStyleSheet("font-size: 24px; color: #0078D7; margin-top: 20px;");
    lblRevenueResult->setAlignment(Qt::AlignCenter);
    revenueLayout->addWidget(lblRevenueResult);
    revenueLayout->addStretch();
    
    tabWidget->addTab(revenueTab, "Фінансовий виторг");

    mainLayout->addWidget(tabWidget);

    // Підключення сигналів
    connect(btnCalcRevenue, &QPushButton::clicked, this, &ReportsTab::onCalculateRevenueClicked);
    connect(btnExportActive, &QPushButton::clicked, this, &ReportsTab::onExportActiveClicked);
    connect(btnExportOverdue, &QPushButton::clicked, this, &ReportsTab::onExportOverdueClicked);
    connect(btnExportWorkload, &QPushButton::clicked, this, &ReportsTab::onExportWorkloadClicked);
}

void ReportsTab::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    refreshReports(); // Автоматичне оновлення при відкритті вкладки
}

void ReportsTab::refreshReports() {
    // 1. Активні замовлення
    activeModel->removeRows(0, activeModel->rowCount());
    auto activeData = ReportBuilder::getActiveOrders();
    for (const auto& row : activeData) {
        QList<QStandardItem*> items;
        items << new QStandardItem(row["order_id"].toString());
        items << new QStandardItem(row["client_name"].toString());
        items << new QStandardItem(row["master_name"].toString());
        items << new QStandardItem(row["status_name"].toString());
        items << new QStandardItem(row["required_date"].toString());
        activeModel->appendRow(items);
    }

    // 2. Прострочені замовлення
    overdueModel->removeRows(0, overdueModel->rowCount());
    auto overdueData = ReportBuilder::getOverdueOrders();
    for (const auto& row : overdueData) {
        QList<QStandardItem*> items;
        items << new QStandardItem(row["order_id"].toString());
        items << new QStandardItem(row["client_name"].toString());
        items << new QStandardItem(row["required_date"].toString());
        items << new QStandardItem(row["status_name"].toString());
        overdueModel->appendRow(items);
    }

    // 3. Завантаженість майстрів
    workloadModel->removeRows(0, workloadModel->rowCount());
    auto workloadData = ReportBuilder::getMasterWorkload();
    for (const auto& row : workloadData) {
        QList<QStandardItem*> items;
        items << new QStandardItem(row["master_name"].toString());
        items << new QStandardItem(row["active_orders"].toString());
        workloadModel->appendRow(items);
    }
}

void ReportsTab::onCalculateRevenueClicked() {
    if (startDateEdit->date() > endDateEdit->date()) {
        QMessageBox::warning(this, "Помилка періоду", "Початкова дата не може бути більшою за кінцеву.");
        return;
    }
    double revenue = ReportBuilder::getRevenueForPeriod(startDateEdit->date(), endDateEdit->date());
    lblRevenueResult->setText(QString("Виторг за обраний період: <b>%1 грн</b>").arg(revenue, 0, 'f', 2));
}

// ==========================================
// ЛОГІКА ЕКСПОРТУ В CSV
// ==========================================

void ReportsTab::exportModelToCsv(QStandardItemModel *model, const QString &defaultFileName) {
    if (model->rowCount() == 0) {
        QMessageBox::information(this, "Порожньо", "Немає даних для експорту.");
        return;
    }

    // Виклик стандартного вікна збереження файлу ОС
    QString fileName = QFileDialog::getSaveFileName(this, "Зберегти звіт у CSV", defaultFileName, "CSV Files (*.csv)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        
        // Встановлюємо кодування UTF-8, щоб українська мова в Excel відображалася коректно
        out.setEncoding(QStringConverter::Utf8);
        
        // Додаємо BOM (Byte Order Mark) для Excel
        out << "\xEF\xBB\xBF"; 

        // 1. Записуємо заголовки колонок
        for (int i = 0; i < model->columnCount(); ++i) {
            out << "\"" << model->horizontalHeaderItem(i)->text() << "\"";
            if (i < model->columnCount() - 1) out << ";"; // Excel в нашому регіоні краще розуміє крапку з комою
        }
        out << "\n";

        // 2. Записуємо дані
        for (int row = 0; row < model->rowCount(); ++row) {
            for (int col = 0; col < model->columnCount(); ++col) {
                out << "\"" << model->item(row, col)->text() << "\"";
                if (col < model->columnCount() - 1) out << ";";
            }
            out << "\n";
        }
        file.close();
        QMessageBox::information(this, "Успіх", "Звіт успішно збережено у файл!");
    } else {
        QMessageBox::critical(this, "Помилка", "Не вдалося створити файл для запису.");
    }
}

void ReportsTab::onExportActiveClicked() {
    exportModelToCsv(activeModel, "Активні_замовлення.csv");
}

void ReportsTab::onExportOverdueClicked() {
    exportModelToCsv(overdueModel, "Прострочені_замовлення.csv");
}

void ReportsTab::onExportWorkloadClicked() {
    exportModelToCsv(workloadModel, "Завантаженість_майстрів.csv");
}