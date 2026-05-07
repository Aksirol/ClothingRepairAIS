#ifndef CLIENTSTAB_H
#define CLIENTSTAB_H

#include <QWidget>
#include <QTableView>
#include <QPushButton>
#include <QLineEdit>
#include <QSqlTableModel>
#include <QSortFilterProxyModel>

class ClientsTab : public QWidget {
    Q_OBJECT
public:
    explicit ClientsTab(QWidget *parent = nullptr);

private slots:
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onSearchTextChanged(const QString &text);

private:
    void setupUi();
    void setupModel();

    QTableView *tableView;
    QSqlTableModel *model;
    QSortFilterProxyModel *proxyModel;
};

#endif