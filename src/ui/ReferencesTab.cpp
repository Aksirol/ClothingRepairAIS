#include "ReferencesTab.h"
#include <QVBoxLayout>
#include <QTabWidget>

// Підключаємо всі наші довідники
#include "CategoriesTab.h"
#include "PositionsTab.h"
#include "StatusesTab.h"

ReferencesTab::ReferencesTab(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    QTabWidget *tabWidget = new QTabWidget(this);
    
    tabWidget->addTab(new CategoriesTab(this), "Категорії послуг");
    tabWidget->addTab(new PositionsTab(this), "Посади");
    tabWidget->addTab(new StatusesTab(this), "Статуси замовлень");

    layout->addWidget(tabWidget);
}