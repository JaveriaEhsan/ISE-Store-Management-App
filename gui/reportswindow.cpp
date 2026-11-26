#include "reportswindow.h"

#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStringList>
#include <QTableWidget>
#include <QVBoxLayout>

enum ReportIndex
{
    REPORT_INCOMPLETE = 0,
    REPORT_COMPLETED,
    REPORT_CASH,
    REPORT_ALL_SALES,
    REPORT_TOP_SOLD,
    REPORT_TOP_STOCK
};

ReportsWindow::ReportsWindow(const ReportService& service, QWidget* parent)
    : QDialog(parent), service(service), reportCombo(nullptr), cashControls(nullptr),
      periodCombo(nullptr), fromEdit(nullptr), toEdit(nullptr), showButton(nullptr),
      includeEditedCheck(nullptr), titleLabel(nullptr), messageLabel(nullptr),
      totalsLabel(nullptr), table(nullptr)
{
    setupUI();
}

void ReportsWindow::setupUI()
{
    setWindowTitle("SmartPOS - Reports");
    resize(900, 600);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(12);

    QLabel* heading = new QLabel("Reports");
    heading->setStyleSheet("font-size: 28px; font-weight: bold; color: #101A35;");

    reportCombo = new QComboBox;
    reportCombo->addItem("Incomplete orders");
    reportCombo->addItem("Completed orders");
    reportCombo->addItem("Cash sales");
    reportCombo->addItem("All sales");
    reportCombo->addItem("Top 10 sold items");
    reportCombo->addItem("Top 10 items in stock");
    reportCombo->setMinimumHeight(36);

    // controls used only by the cash sales report
    cashControls = new QWidget;
    QHBoxLayout* cashLayout = new QHBoxLayout(cashControls);
    cashLayout->setContentsMargins(0, 0, 0, 0);

    periodCombo = new QComboBox;
    periodCombo->addItem("Daily");
    periodCombo->addItem("Weekly");
    periodCombo->addItem("Monthly");
    periodCombo->addItem("Yearly");
    periodCombo->addItem("Custom");

    fromEdit = new QLineEdit;
    fromEdit->setPlaceholderText("From (yyyy-MM-dd)");
    toEdit = new QLineEdit;
    toEdit->setPlaceholderText("To (yyyy-MM-dd)");
    showButton = new QPushButton("Show");

    cashLayout->addWidget(new QLabel("Period:"));
    cashLayout->addWidget(periodCombo);
    cashLayout->addWidget(fromEdit);
    cashLayout->addWidget(toEdit);
    cashLayout->addWidget(showButton);

    includeEditedCheck = new QCheckBox("Include edited bills");

    titleLabel = new QLabel;
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold;");

    messageLabel = new QLabel;
    messageLabel->setStyleSheet("color: #B00020;");
    messageLabel->hide();

    table = new QTableWidget;
    table->verticalHeader()->setVisible(false);      // new: no second count
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);
    table->horizontalHeader()->setStretchLastSection(true);
    
    totalsLabel = new QLabel;
    totalsLabel->setStyleSheet("font-size: 14px; font-weight: bold;");

    layout->addWidget(heading);
    layout->addWidget(reportCombo);
    layout->addWidget(cashControls);
    layout->addWidget(includeEditedCheck);
    layout->addWidget(titleLabel);
    layout->addWidget(messageLabel);
    layout->addWidget(table, 1);
    layout->addWidget(totalsLabel);

    connect(reportCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ReportsWindow::showSelectedReport);
    connect(periodCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ReportsWindow::periodChanged);
    connect(showButton, &QPushButton::clicked, this, &ReportsWindow::showSelectedReport);
    connect(includeEditedCheck, &QCheckBox::toggled, this, &ReportsWindow::showSelectedReport);

    periodChanged();            // fills the dates for "Daily"
    showSelectedReport();
}

// Shows only the controls that the chosen report needs.
void ReportsWindow::updateControls()
{
    int index = reportCombo->currentIndex();
    cashControls->setVisible(index == REPORT_CASH);
    includeEditedCheck->setVisible(index == REPORT_ALL_SALES);
}

// Fills the date boxes from the period. "Custom" lets the user type the dates.
void ReportsWindow::periodChanged()
{
    int p = periodCombo->currentIndex();
    bool custom = (p == 4);

    fromEdit->setReadOnly(!custom);
    toEdit->setReadOnly(!custom);

    if (!custom)
    {
        std::string from, to;
        ReportService::periodRange(static_cast<ReportPeriod>(p), from, to);
        fromEdit->setText(QString::fromStdString(from));
        toEdit->setText(QString::fromStdString(to));
        if (reportCombo->currentIndex() == REPORT_CASH)
            showSelectedReport();
    }
}

void ReportsWindow::showSelectedReport()
{
    updateControls();
    messageLabel->hide();

    switch (reportCombo->currentIndex())
    {
    case REPORT_INCOMPLETE:
        showTable(service.incompleteOrders());
        break;
    case REPORT_COMPLETED:
        showTable(service.completedOrders());
        break;
    case REPORT_CASH:
    {
        ReportTable report;
        std::string error;
        if (service.cashSales(fromEdit->text().toStdString(),
                              toEdit->text().toStdString(), report, error))
            showTable(report);
        else
            showError(QString::fromStdString(error));
        break;
    }
    case REPORT_ALL_SALES:
        showTable(service.allSales(includeEditedCheck->isChecked()));
        break;
    case REPORT_TOP_SOLD:
        showTable(service.topSoldItems());
        break;
    case REPORT_TOP_STOCK:
        showTable(service.topStockItems());
        break;
    }
}

// Generic: displays any ReportTable, so a new report needs no change here.
void ReportsWindow::showTable(const ReportTable& report)
{
    titleLabel->setText(QString::fromStdString(report.title));

    table->clear();
    table->setRowCount(0);
    table->setColumnCount(static_cast<int>(report.headers.size()));

    QStringList headers;
    for (size_t c = 0; c < report.headers.size(); c++)
        headers << QString::fromStdString(report.headers[c]);
    table->setHorizontalHeaderLabels(headers);

    table->setRowCount(static_cast<int>(report.rows.size()));
    for (size_t r = 0; r < report.rows.size(); r++)
    {
        for (size_t c = 0; c < report.rows[r].size() && c < report.headers.size(); c++)
        {
            table->setItem(static_cast<int>(r), static_cast<int>(c),
                new QTableWidgetItem(QString::fromStdString(report.rows[r][c])));
        }
    }
    table->resizeColumnsToContents();
    table->horizontalHeader()->setStretchLastSection(true);

    QStringList totals;
    for (size_t i = 0; i < report.totals.size(); i++)
    {
        totals << QString::fromStdString(report.totals[i].label) + ": "
                  + QString::number(report.totals[i].value, 'f', 2);
    }
    totalsLabel->setText(totals.join("     "));
}

void ReportsWindow::showError(const QString& message)
{
    titleLabel->clear();
    table->clear();
    table->setRowCount(0);
    table->setColumnCount(0);
    totalsLabel->clear();
    messageLabel->setText(message);
    messageLabel->show();
}