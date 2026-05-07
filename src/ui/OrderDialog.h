#ifndef ORDERDIALOG_H
#define ORDERDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QTextEdit>
#include <QTableView>
#include <QStandardItemModel>
#include <QLabel>
#include <QPushButton>
#include "Order.h"
#include "OrderItem.h"

class OrderDialog : public QDialog {
    Q_OBJECT
public:
    explicit OrderDialog(QWidget *parent = nullptr);
    std::vector<OrderItem> getOrderItemsData() const;

    // Метод для отримання заповненої структури замовлення
    Order getOrderData() const;

private slots:
    void onAddClientClicked();
    void onAddItemClicked();
    void onRemoveItemClicked();
    void onSaveClicked();

private:
    void setupUi();
    void populateClients();
    void populateEmployees();
    void calculateTotal();

    QComboBox *clientCombo;
    QPushButton *btnAddClient;
    QComboBox *employeeCombo;
    QDateEdit *receivedDateEdit;
    QDateEdit *requiredDateEdit;
    QDoubleSpinBox *depositSpin;
    QTextEdit *notesEdit;
    QTableView *itemsTable;
    QStandardItemModel *itemsModel;
    QLabel *lblTotalSum;
};

#endif // ORDERDIALOG_H