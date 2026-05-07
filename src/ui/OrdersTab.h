#ifndef ORDERSTAB_H
#define ORDERSTAB_H

#include <QWidget>
#include <QTableView>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QSqlQueryModel>
#include <QSortFilterProxyModel>

class OrdersTab : public QWidget {
    Q_OBJECT
public:
    explicit OrdersTab(QWidget *parent = nullptr);

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void onAddOrderClicked();
    void onEditOrderClicked();
    void onDeleteOrderClicked();
    void onSearchTextChanged(const QString &text);
    void onStatusFilterChanged(int index);
    void refreshData(); // Оновлення SQL-запиту

private:
    void setupUi();
    void setupModel();

    QTableView *tableView;
    QLineEdit *searchEdit;
    QComboBox *statusFilterCombo;
    
    QSqlQueryModel *model;
    QSortFilterProxyModel *proxyModel;
    
    int colorColumnIndex = 8; // Індекс колонки з кольором у нашому SQL-запиті
};

#endif