#ifndef CATEGORIESTAB_H
#define CATEGORIESTAB_H

#include <QWidget>
#include <QTableView>
#include <QPushButton>
#include <QLineEdit>
#include <QSqlTableModel>
#include <QSortFilterProxyModel>

class CategoriesTab : public QWidget {
    Q_OBJECT

public:
    explicit CategoriesTab(QWidget *parent = nullptr);

private slots:
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onSearchTextChanged(const QString &text);

private:
    void setupUi();
    void setupModel();

    QTableView *tableView;
    QPushButton *btnAdd;
    QPushButton *btnEdit;
    QPushButton *btnDelete;
    QLineEdit *searchEdit;

    QSqlTableModel *model;
    QSortFilterProxyModel *proxyModel;
};

#endif // CATEGORIESTAB_H