#ifndef REPORTSTAB_H
#define REPORTSTAB_H

#include <QWidget>
#include <QStandardItemModel>
#include <QDateEdit>
#include <QLabel>

class ReportsTab : public QWidget {
    Q_OBJECT

public:
    explicit ReportsTab(QWidget *parent = nullptr);

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void onCalculateRevenueClicked();
    void onExportActiveClicked();
    void onExportOverdueClicked();
    void onExportWorkloadClicked();

private:
    void setupUi();
    void refreshReports();
    void exportModelToCsv(QStandardItemModel *model, const QString &defaultFileName);

    // Моделі для таблиць
    QStandardItemModel *activeModel;
    QStandardItemModel *overdueModel;
    QStandardItemModel *workloadModel;

    // Віджети для виторгу
    QDateEdit *startDateEdit;
    QDateEdit *endDateEdit;
    QLabel *lblRevenueResult;
};

#endif // REPORTSTAB_H