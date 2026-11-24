#include "ApprovalRequestsWindow.h"
#include "../data/ApprovalRepository.h"
#include "../data/OrderRepository.h"
#include "../services/OrderService.h"
#include "../models/approval.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

using namespace std;

ApprovalRequestsWindow::ApprovalRequestsWindow(ApprovalRepository& approvals, OrderRepository& orders, QWidget* parent)
    : QDialog(parent), approvals(approvals), orders(orders)
{
    setWindowTitle("Approval requests");
    resize(760, 420);

    table = new QTableWidget(0, 6, this);
    table->setHorizontalHeaderLabels({ "Request", "Order", "Customer", "Order date",
                                       "Amount", "Requested at" });
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->setVisible(false);

    emptyLabel = new QLabel("No pending requests.", this);
    emptyLabel->setAlignment(Qt::AlignCenter);

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
    layout->addLayout(buttons);

    connect(approveButton, &QPushButton::clicked, this, &ApprovalRequestsWindow::approveSelected);
    connect(rejectButton, &QPushButton::clicked, this, &ApprovalRequestsWindow::rejectSelected);
    connect(refreshButton, &QPushButton::clicked, this, &ApprovalRequestsWindow::refresh);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    refresh();      // load when the window opens
}

void ApprovalRequestsWindow::refresh()
{
    vector<Approval> pending = approvals.findByStatus(APPROVAL_PENDING);

    table->setRowCount(0);
    for (const Approval& a : pending)
    {
        Order o;
        bool haveOrder = orders.findById(a.orderId, o);

        int row = table->rowCount();
        table->insertRow(row);

        QString customer = haveOrder ? QString::fromStdString(o.customerName).replace('_', ' ')
                                     : QString("(order not found)");
        QString orderDate = haveOrder ? QString::fromStdString(o.date) : QString("-");
        QString amount = haveOrder ? QString::number(o.paymentAmount, 'f', 2) : QString("-");

        table->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(a.requestId)));
        table->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(a.orderId)));
        table->setItem(row, 2, new QTableWidgetItem(customer));
        table->setItem(row, 3, new QTableWidgetItem(orderDate));
        table->setItem(row, 4, new QTableWidgetItem(amount));
        table->setItem(row, 5, new QTableWidgetItem(
            QString::fromStdString(a.requestDate + " " + a.requestTime)));
    }

    bool any = table->rowCount() > 0;
    table->setVisible(any);
    emptyLabel->setVisible(!any);
}

string ApprovalRequestsWindow::selectedRequestId() const
{
    int row = table->currentRow();
    if (row < 0 || !table->item(row, 0))
    {
        return "";
    }
    return table->item(row, 0)->text().toStdString();
}

void ApprovalRequestsWindow::approveSelected()
{
    decide(APPROVAL_APPROVED);
}

void ApprovalRequestsWindow::rejectSelected()
{
    decide(APPROVAL_REJECTED);
}

void ApprovalRequestsWindow::decide(const string& newStatus)
{
    string requestId = selectedRequestId();
    if (requestId.empty())
    {
        QMessageBox::information(this, "Approval requests", "Select a request first.");
        return;
    }

    // an approval is pointless if the 7 days have already passed
    if (newStatus == APPROVAL_APPROVED)
    {
        for (const Approval& a : approvals.findByStatus(APPROVAL_PENDING))
        {
            if (a.requestId != requestId) continue;

            Order o;
            string reason;
            if (!orders.findById(a.orderId, o))
            {
                QMessageBox::warning(this, "Approval requests", "The order no longer exists.");
                return;
            }
            if (!OrderService::canRequestEdit(o, reason))
            {
                QMessageBox::warning(this, "Approval requests",
                                     QString::fromStdString(reason) + "\nYou can only reject it.");
                return;
            }
        }
    }

    string error;
    if (!approvals.setStatus(requestId, newStatus, error))
    {
        QMessageBox::warning(this, "Approval requests", QString::fromStdString(error));
    }
    refresh();      // the decided request leaves the pending list
}