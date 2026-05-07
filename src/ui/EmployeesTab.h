#ifndef EMPLOYEESTAB_H
#define EMPLOYEESTAB_H

#include <QWidget>
#include <QTableView>
#include <QPushButton>
#include <QLineEdit>
#include <QSqlRelationalTableModel>
#include <QSortFilterProxyModel>

class EmployeesTab : public QWidget {
    Q_OBJECT
public:
    explicit EmployeesTab(QWidget *parent = nullptr);

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onSearchTextChanged(const QString &text);

private:
    void setupUi();
    void setupModel();
    void populatePositionsCombo(class QComboBox *combo);

    QTableView *tableView;
    QSqlRelationalTableModel *model;
    QSortFilterProxyModel *proxyModel;
};

#endif