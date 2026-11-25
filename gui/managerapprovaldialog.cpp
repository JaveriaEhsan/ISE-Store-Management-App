#include "managerapprovaldialog.h"
#include "data/ApprovalRepository.h"
#include "data/OrderRepository.h"
#include "data/OrderDetailRepository.h"
#include "services/OrderService.h"
#include "models/approval.h"

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

ManagerApprovalDialog::ManagerApprovalDialog(ApprovalRepository& approvals,
                                             OrderRepository& orders,
                                             OrderDetailRepository& orderDetails,
                                             QWidget* parent)
    : QDialog(parent), approvals(approvals), orders(orders), orderDetails(orderDetails)
{
    setWindowTitle("Approve editing of paid orders");
    resize(780, 520);

    // ----- table of pending requests -----
    table = new QTableWidget(0, 6, this);
    table->setHorizontalHeaderLabels({ "Request", "Order", "Customer",
                                       "Order date", "Amount", "Requested at" });
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);

    emptyLabel = new QLabel("No pending requests.", this);
    emptyLabel->setAlignment(Qt::AlignCenter);

    // ----- items of the selected order -----
    itemsLabel = new QLabel("Select a request to see the items of the order.", this);
    itemsLabel->setWordWrap(true);
    itemsLabel->setFrameStyle(QFrame::StyledPanel);
    itemsLabel->setMargin(8);

    // ----- buttons -----
    QPushButton* approveButton = new QPushButton("Approve", this);
    QPushButton* rejectButton = new QPushButton("Reject", this);
    QPushButton* refreshButton = new QPushButton("Refresh", this);
    QPushButton* closeButton = new QPushButton("Close", this);

    QHBoxLayout* buttons = new QHBoxLayout();
    buttons->addWidget(approveButton);
    buttons->addWidget(rejectButton);
    buttons->addStretch();
    buttons->addWidget(refreshButton);
    buttons->addWidget(closeButton);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(table);
    layout->addWidget(emptyLabel);
    layout->addWidget(itemsLabel);
    layout->addLayout(buttons);

    connect(approveButton, &QPushButton::clicked, this, &ManagerApprovalDialog::approveClicked);
    connect(rejectButton, &QPushButton::clicked, this, &ManagerApprovalDialog::rejectClicked);
    connect(refreshButton, &QPushButton::clicked, this, &ManagerApprovalDialog::refresh);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(table, &QTableWidget::itemSelectionChanged, this, &ManagerApprovalDialog::showItems);

    refresh();     // load when the window opens
}

void ManagerApprovalDialog::refresh()
{
    table->setRowCount(0);

    for (const Approval& a : approvals.findByStatus(APPROVAL_PENDING))
    {
        Order o;
        bool found = orders.findById(a.orderId, o);

        QString customer = "(order not found)";
        QString date = "-";
        QString amount = "-";
        if (found)
        {
            customer = QString::fromStdString(o.customerName);
            customer.replace('_', ' ');
            date = QString::fromStdString(o.date);
            amount = QString::number(o.paymentAmount, 'f', 2);
        }

        int row = table->rowCount();
        table->insertRow(row);
        table->setItem(row, 0, cell(QString::fromStdString(a.requestId)));
        table->setItem(row, 1, cell(QString::fromStdString(a.orderId)));
        table->setItem(row, 2, cell(customer));
        table->setItem(row, 3, cell(date));
        table->setItem(row, 4, cell(amount));
        table->setItem(row, 5, cell(QString::fromStdString(a.requestDate + " " + a.requestTime)));
    }

    bool any = table->rowCount() > 0;
    table->setVisible(any);
    emptyLabel->setVisible(!any);
    itemsLabel->setText("Select a request to see the items of the order.");
}

string ManagerApprovalDialog::selectedRequestId() const
{
    int row = table->currentRow();
    if (row < 0 || table->selectedItems().isEmpty() || !table->item(row, 0)) return "";
    return table->item(row, 0)->text().toStdString();
}

string ManagerApprovalDialog::selectedOrderId() const
{
    int row = table->currentRow();
    if (row < 0 || table->selectedItems().isEmpty() || !table->item(row, 1)) return "";
    return table->item(row, 1)->text().toStdString();
}

void ManagerApprovalDialog::showItems()
{
    string orderId = selectedOrderId();
    if (orderId.empty())
    {
        itemsLabel->setText("Select a request to see the items of the order.");
        return;
    }

    QString text = "Items of order " + QString::fromStdString(orderId) + ":\n";
    vector<OrderDetail> lines = orderDetails.findByOrderId(orderId);
    if (lines.empty())
    {
        text += "(no items found)";
    }
    for (const OrderDetail& d : lines)
    {
        text += "  " + QString::fromStdString(d.barcode) + "   x " +
                QString::number(d.quantity, 'g', 10) + "\n";
    }
    itemsLabel->setText(text);
}

void ManagerApprovalDialog::approveClicked()
{
    decide(APPROVAL_APPROVED);
}

void ManagerApprovalDialog::rejectClicked()
{
    decide(APPROVAL_REJECTED);
}

void ManagerApprovalDialog::decide(const string& newStatus)
{
    string requestId = selectedRequestId();
    string orderId = selectedOrderId();
    if (requestId.empty())
    {
        QMessageBox::information(this, "Approval", "Select a request first.");
        return;
    }

    // approving is pointless if the order is no longer inside the 7 days
    if (newStatus == APPROVAL_APPROVED)
    {
        Order o;
        if (!orders.findById(orderId, o))
        {
            QMessageBox::warning(this, "Approval", "The order no longer exists. You can only reject it.");
            return;
        }
        string reason;
        if (!OrderService::canRequestEdit(o, reason))
        {
            QMessageBox::warning(this, "Approval",
                QString::fromStdString(reason) + "\nYou can only reject it.");
            return;
        }
    }

    string error;
    if (!approvals.setStatus(requestId, newStatus, error))
    {
        QMessageBox::warning(this, "Approval", QString::fromStdString(error));
    }
    refresh();     // the decided request leaves the pending list
}