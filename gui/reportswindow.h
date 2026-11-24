#ifndef REPORTSWINDOW_H
#define REPORTSWINDOW_H

#include <QDialog>
#include "../services/ReportService.h"

class QComboBox;
class QTableWidget;
class QLineEdit;
class QCheckBox;
class QLabel;
class QPushButton;
class QWidget;

// Shows one report at a time. The window only displays data,
// all calculations stay in ReportService.
class ReportsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ReportsWindow(const ReportService& service, QWidget* parent = nullptr);

private slots:
    void showSelectedReport();
    void periodChanged();

private:
    void setupUI();
    void updateControls();
    void showTable(const ReportTable& report);
    void showError(const QString& message);

    const ReportService& service;
    QComboBox* reportCombo;
    QWidget* cashControls;
    QComboBox* periodCombo;
    QLineEdit* fromEdit;
    QLineEdit* toEdit;
    QPushButton* showButton;
    QCheckBox* includeEditedCheck;
    QLabel* titleLabel;
    QLabel* messageLabel;
    QLabel* totalsLabel;
    QTableWidget* table;
};

#endif