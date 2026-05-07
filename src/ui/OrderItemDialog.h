#ifndef ORDERITEMDIALOG_H
#define ORDERITEMDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLineEdit>

class OrderItemDialog : public QDialog {
    Q_OBJECT
public:
    explicit OrderItemDialog(QWidget *parent = nullptr);

    int getServiceId() const;
    QString getServiceName() const;
    int getQuantity() const;
    double getUnitPrice() const;
    QString getDescription() const;

private slots:
    void onCategoryChanged();
    void onServiceChanged();

private:
    void setupUi();
    void loadCategories();

    QComboBox *categoryCombo;
    QComboBox *serviceCombo;
    QSpinBox *quantitySpin;
    QDoubleSpinBox *priceSpin;
    QLineEdit *descriptionEdit;
};

#endif