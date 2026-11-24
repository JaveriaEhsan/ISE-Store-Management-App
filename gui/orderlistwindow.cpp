#include "orderlistwindow.h"
#include "data/OrderRepository.h"
#include "data/ApprovalRepository.h"
#include "models/approval.h"
#include "models/order.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

using namespace std;

static QTableWidgetItem* cell(const QString& text)
{
    QTableWidgetItem* item = new QTableWidgetItem(text);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    return item;
}

OrderListWindow::OrderListWindow(OrderRepository& orders, ApprovalRepository& approvals,
                                 QWidget* parent)
    : QDialog(parent), orders(orders), approvals(approvals)
{
    setWindowTitle("Edit existing order");
    resize(760, 430);

    QLabel* info = new QLabel(
        QString("Paid orders of the last %1 days. A paid order needs the manager's approval "
                "before it can be edited.").arg(PAID_ORDER_EDIT_DAYS), this);
    info->setWordWrap(true);

    table = new QTableWidget(0, 5, this);
    table->setHorizontalHeaderLabels({ "Order ID", "Customer", "Date", "Amount", "Approval" });
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);

    requestButton = new QPushButton("Request Approval", this);
    editButton = new QPushButton("Edit", this);
    QPushButton* refreshButton = new QPushButton("Refresh", this);
    QPushButton* closeButton = new QPushButton("Close", this);

    QHBoxLayout* buttons = new QHBoxLayout();
    buttons->addWidget(requestButton);
    buttons->addWidget(editButton);
    buttons->addStretch();
    buttons->addWidget(refreshButton);
    buttons->addWidget(closeButton);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(info);
    layout->addWidget(table);
    layout->addLayout(buttons);

    connect(requestButton, &QPushButton::clicked, this, &OrderListWindow::requestSelected);
    connect(editButton, &QPushButton::clicked, this, &OrderListWindow::editSelected);
    connect(refreshButton, &QPushButton::clicked, this, &OrderListWindow::refresh);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(table, &QTableWidget::itemSelectionChanged, this, &OrderListWindow::updateButtons);

    refresh();
}

// The newest request of this order decides what the cashier sees
string OrderListWindow::statusOf(const string& orderId) const
{
    vector<Approval> list = approvals.findByOrderId(orderId);   // in saving order
    if (list.empty()) return "none";
    return list.back().status;
}

void OrderListWindow::refresh()
{
    table->setRowCount(0);

    vector<Order> paid = orders.findByStatus(STATUS_PAID);
    for (size_t i = paid.size(); i > 0; i--)          // newest first
    {
        const Order& o = paid[i - 1];

        string reason;
        if (!OrderService::canRequestEdit(o, reason)) continue;   // older than 7 days

        string status = statusOf(o.orderId);
        QString statusText;
        if (status == APPROVAL_PENDING)       statusText = "Pending";
        else if (status == APPROVAL_APPROVED) statusText = "Approved";
        else if (status == APPROVAL_REJECTED) statusText = "Rejected";
        else if (status == APPROVAL_USED)     statusText = "Edit done";
        else                                  statusText = "-";

        QString customer = QString::fromStdString(o.customerName);
        customer.replace('_', ' ');

        int row = table->rowCount();
        table->insertRow(row);
        table->setItem(row, 0, cell(QString::fromStdString(o.orderId)));
        table->setItem(row, 1, cell(customer));
        table->setItem(row, 2, cell(QString::fromStdString(o.date)));
        table->setItem(row, 3, cell(QString::number(o.paymentAmount, 'f', 2)));
        table->setItem(row, 4, cell(statusText));
    }
    updateButtons();
}

string OrderListWindow::currentId() const
{
    int row = table->currentRow();
    if (row < 0 || !table->item(row, 0) || table->selectedItems().isEmpty()) return "";
    return table->item(row, 0)->text().toStdString();
}

void OrderListWindow::updateButtons()
{
    string id = currentId();
    string status = id.empty() ? "" : statusOf(id);

    // a new request is allowed when none is waiting or approved
    requestButton->setEnabled(!id.empty() && status != APPROVAL_PENDING && status != APPROVAL_APPROVED);
    editButton->setEnabled(!id.empty() && status == APPROVAL_APPROVED);
}

void OrderListWindow::requestSelected()
{
    string id = currentId();
    if (id.empty()) return;

    string requestId, error;
    if (!service.requestApproval(id, orders, approvals, requestId, error))
    {
        QMessageBox::warning(this, "Request approval", QString::fromStdString(error));
    }
    else
    {
        QMessageBox::information(this, "Request approval",
            "The request was sent to the manager.\nCome back here after it is approved.");
    }
    refresh();
}

void OrderListWindow::editSelected()
{
    string id = currentId();
    if (id.empty()) return;

    if (statusOf(id) != APPROVAL_APPROVED)
    {
        QMessageBox::information(this, "Edit order", "The manager has not approved this order yet.");
        refresh();
        return;
    }

    chosenId = id;
    accept();          // CashierWindow loads the order (the service checks the approval again)
}