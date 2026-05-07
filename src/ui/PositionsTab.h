#ifndef POSITIONSTAB_H
#define POSITIONSTAB_H

#include <QWidget>
#include <QTableView>
#include <QPushButton>
#include <QLineEdit>
#include <QSqlTableModel>
#include <QSortFilterProxyModel>

class PositionsTab : public QWidget {
    Q_OBJECT
public:
    explicit PositionsTab(QWidget *parent = nullptr);

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

    QTableView *tableView;
    QSqlTableModel *model;
    QSortFilterProxyModel *proxyModel;
};

#endif