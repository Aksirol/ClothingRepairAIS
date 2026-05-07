#ifndef CATEGORIESTAB_H
#define CATEGORIESTAB_H

#include <QWidget>
#include <QTableView>
#include <QSqlTableModel>
#include <QSortFilterProxyModel>

class CategoriesTab : public QWidget {
    Q_OBJECT

public:
    explicit CategoriesTab(QWidget *parent = nullptr);

protected:
    void showEvent(QShowEvent *event) override; // Додано для оновлення при відкритті

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

#endif // CATEGORIESTAB_H