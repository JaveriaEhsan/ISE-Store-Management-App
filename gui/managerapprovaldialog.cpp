#include "managerapprovaldialog.h"
#include "models/approval.h"
#include "models/order.h"
#include "models/bill.h"
#include "models/billDetail.h"
#include "services/OrderService.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <vector>

using namespace std;

static QTableWidgetItem* cell(const QString& text)
{
    QTableWidgetItem* item = new QTableWidgetItem(text);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    item->setTextAlignment(Qt::AlignCenter);
    return item;
}

ManagerApprovalDialog::ManagerApprovalDialog(ApprovalRepository& approvals,
                                             OrderRepository& orders,
                                             OrderDetailRepository& orderDetails,
                                             IProductRepository& products,
                                             BillRepository& bills,
                                             BillDetailRepository& billDetails,
                                             QWidget* parent)
    : QDialog(parent), approvals(approvals), orders(orders), orderDetails(orderDetails),
      products(products), bills(bills), billDetails(billDetails)
{
    setWindowTitle("Approve editing of paid orders");
    resize(860, 640);

    // ----- table of pending requests -----
    table = new QTableWidget(0, 6, this);
    table->setHorizontalHeaderLabels({ "Request", "Order", "Customer",
                                       "Order date", "Amount", "Requested at" });
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);

    emptyLabel = new QLabel("No pending requests.", this);
    emptyLabel->setAlignment(Qt::AlignCenter);

    // ----- details of the selected order -----
    itemsLabel = new QTextEdit(this);
    itemsLabel->setReadOnly(true);
    itemsLabel->setMinimumHeight(200);
    itemsLabel->setHtml("Select a request to see the items of the order.");

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
    layout->addWidget(table, 1);
    layout->addWidget(emptyLabel);
    layout->addWidget(itemsLabel, 1);
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
    itemsLabel->setHtml("Select a request to see the items of the order.");
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

// Shows what was sold in the selected order
void ManagerApprovalDialog::showItems()
{
    string orderId = selectedOrderId();
    if (orderId.empty())
    {
        itemsLabel->setHtml("Select a request to see the items of the order.");
        return;
    }

    Order o;
    orders.findById(orderId, o);

    QString html = "<b>Order " + QString::fromStdString(orderId) + "</b>";
    if (!o.orderId.empty())
    {
        QString customer = QString::fromStdString(o.customerName);
        customer.replace('_', ' ');
        html += " &nbsp;|&nbsp; " + customer + " &nbsp;|&nbsp; " +
                QString::fromStdString(o.date + " " + o.time);
    }

    html += "<table border='1' cellspacing='0' cellpadding='4' width='100%'>"
            "<tr style='background:#EEF2FA'>"
            "<th>Product</th><th>Qty</th><th>Unit price</th>"
            "<th>Discount</th><th>Line total</th></tr>";

    // 1. the original bill of this order (what the customer was really charged)
    Bill bill;
    vector<BillDetail> lines;
    bool fromBill = false;
    if (bills.findCurrentByOrderId(orderId, bill))
    {
        lines = billDetails.findByBillId(bill.billId);
        fromBill = !lines.empty();
    }

    if (fromBill)
    {
        for (const BillDetail& d : lines)
        {
            // BillDetail stores only the acronym, so the name comes from the product
            Product p;
            QString name = QString::fromStdString(d.acronym);
            for (const Product& candidate : products.getAll())
            {
                if (candidate.getAcronym() == d.acronym)
                {
                    name = QString::fromStdString(candidate.getName()) +
                           " (" + QString::fromStdString(d.acronym) + ")";
                    break;
                }
            }
            double lineTotal = d.unitPrice * d.quantity - d.discount;   // before tax

            html += "<tr><td>" + name + "</td>"
                    "<td align='center'>" + QString::number(d.quantity, 'g', 10) + "</td>"
                    "<td align='right'>" + QString::number(d.unitPrice, 'f', 2) + "</td>"
                    "<td align='right'>" + QString::number(d.discount, 'f', 2) + "</td>"
                    "<td align='right'>" + QString::number(lineTotal, 'f', 2) + "</td></tr>";
        }
        html += "</table>";

        html += "<p>Bill " + QString::fromStdString(bill.billId) +
                ": without tax " + QString::number(bill.totalWithoutTax, 'f', 2) +
                " &nbsp; product discount " + QString::number(bill.totalDiscount, 'f', 2) +
                " &nbsp; seasonal " + QString::number(bill.seasonalDiscount, 'f', 2) +
                " &nbsp; special " + QString::number(bill.specialDiscount, 'f', 2) +
                " &nbsp; tax " + QString::number(bill.totalTax, 'f', 2) +
                "<br><b>Payable: " + QString::number(bill.totalPayable, 'f', 2) + "</b></p>";
    }
    else
    {
        // 2. no bill lines found: use the order lines and the current prices
        double without = 0, tax = 0, discount = 0;
        for (const OrderDetail& d : orderDetails.findByOrderId(orderId))
        {
            Product p;
            bool known = products.findByBarcode(d.barcode, p);
            QString name = known ? QString::fromStdString(p.getName())
                                 : QString("(removed product)");
            double price = known ? p.getPrice() : 0;
            double t = known ? p.getTax() : 0;
            double disc = known ? p.getDiscount() * d.quantity : 0;
            double lineTotal = price * d.quantity - disc;

            without += price * d.quantity;
            tax += t * d.quantity;
            discount += disc;

            html += "<tr><td>" + name + "</td>"
                    "<td align='center'>" + QString::number(d.quantity, 'g', 10) + "</td>"
                    "<td align='right'>" + QString::number(price, 'f', 2) + "</td>"
                    "<td align='right'>" + QString::number(disc, 'f', 2) + "</td>"
                    "<td align='right'>" + QString::number(lineTotal, 'f', 2) + "</td></tr>";
        }
        html += "</table><p>(no bill found, showing current prices) Without tax: " +
                QString::number(without, 'f', 2) + " &nbsp; Discount: " +
                QString::number(discount, 'f', 2) + " &nbsp; Tax: " +
                QString::number(tax, 'f', 2) + "</p>";
    }

    itemsLabel->setHtml(html);
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
            QMessageBox::warning(this, "Approval",
                "The order no longer exists. You can only reject it.");
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