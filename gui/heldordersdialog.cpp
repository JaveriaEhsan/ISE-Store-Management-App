#include "heldordersdialog.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>

using namespace std;

HeldOrdersDialog::HeldOrdersDialog(const vector<Order>& orders, QWidget* parent)
    : QDialog(parent), held(orders)
{
    setWindowTitle("Held Orders");
    resize(620, 380);

    table = new QTableWidget((int)held.size(), 4, this);
    table->setHorizontalHeaderLabels({ "Order ID", "Customer", "Date / time", "Amount" });
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);

    for (int i = 0; i < (int)held.size(); i++)
    {
        const Order& o = held[i];
        QString name = QString::fromStdString(o.customerName);
        name.replace('_', ' ');
        table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(o.orderId)));
        table->setItem(i, 1, new QTableWidgetItem(name));
        table->setItem(i, 2, new QTableWidgetItem(
            QString::fromStdString(o.date + " " + o.time)));
        table->setItem(i, 3, new QTableWidgetItem(QString::number(o.paymentAmount, 'f', 2)));
    }

    QPushButton* resumeButton = new QPushButton("Resume", this);
    QPushButton* cancelButton = new QPushButton("Cancel", this);
    QHBoxLayout* buttons = new QHBoxLayout();
    buttons->addStretch(1);
    buttons->addWidget(resumeButton);
    buttons->addWidget(cancelButton);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Select an order and press Resume (or double-click it).", this));
    layout->addWidget(table, 1);
    layout->addLayout(buttons);

    connect(resumeButton, &QPushButton::clicked, this, &HeldOrdersDialog::onResume);
    connect(table, &QTableWidget::cellDoubleClicked, this, [this]() { onResume(); });
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void HeldOrdersDialog::onResume()
{
    int row = table->currentRow();
    if (row < 0 || row >= (int)held.size())
    {
        QMessageBox::information(this, "Held orders", "Select an order first.");
        return;
    }
    chosenRow = row;
    accept();
}

Order HeldOrdersDialog::selectedOrder() const
{
    return held[chosenRow];
}