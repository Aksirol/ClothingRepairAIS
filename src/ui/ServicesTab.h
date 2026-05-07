#ifndef SERVICESTAB_H
#define SERVICESTAB_H

#include <QWidget>
#include <QTableView>
#include <QSqlRelationalTableModel>
#include <QSortFilterProxyModel>

class ServicesTab : public QWidget {
    Q_OBJECT
public:
    explicit ServicesTab(QWidget *parent = nullptr);
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
    void populateCategoriesCombo(class QComboBox *combo);

    QTableView *tableView;
    QSqlRelationalTableModel *model;
    QSortFilterProxyModel *proxyModel;
};

#endif