#include "billdialog.h"

#include <string>
#include <vector>

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

using namespace std;

static QString text(const string& s)
{
    return s.empty() ? QString("-") : QString::fromStdString(s);
}

static QString money(double value)
{
    return QString::number(value, 'f', 2);
}

BillDialog::BillDialog(const CheckoutResult& result, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Bill");
    resize(900, 520);

    const Order& o = result.order;
    const Bill& b = result.bill;

    QFormLayout* header = new QFormLayout();
    header->addRow("Order id:", new QLabel(text(o.orderId), this));
    header->addRow("Bill id:", new QLabel(text(b.billId), this));
    header->addRow("Date:", new QLabel(text(o.date), this));
    header->addRow("Time:", new QLabel(text(o.time), this));
    header->addRow("Customer:", new QLabel(text(b.customerName), this));
    header->addRow("Payment mode:", new QLabel(text(o.paymentMode), this));
    header->addRow("Merchant:", new QLabel(text(o.merchant), this));
    header->addRow("Status:", new QLabel(text(b.status), this));

  
       // lines table 
    QTableWidget* table = new QTableWidget((int)b.lines.size(), 8, this);
    table->setHorizontalHeaderLabels(
        { "ID", "Product", "Barcode", "Unit price", "Tax/unit", "Discount", "Quantity", "Total price" });
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    BillCalculator calculator;
    for (size_t i = 0; i < b.lines.size(); i++)
    {
        const BillLine& line = b.lines[i];

        // total= (price - discount + tax) x quantity
        BillItem item;
        item.unitPrice = line.unitPrice;
        item.taxPerUnit = line.taxPerUnit;
        item.discountPerUnit = line.discountPerUnit;
        item.quantity = line.quantity;
        BillTotals lineTotals = calculator.calculate(vector<BillItem>(1, item));

        table->setItem((int)i, 0, new QTableWidgetItem(QString::number(i + 1)));   
        table->setItem((int)i, 1, new QTableWidgetItem(text(line.acronym)));
        table->setItem((int)i, 2, new QTableWidgetItem(text(line.barcode)));
        table->setItem((int)i, 3, new QTableWidgetItem(money(line.unitPrice)));
        table->setItem((int)i, 4, new QTableWidgetItem(money(line.taxPerUnit)));
        table->setItem((int)i, 5, new QTableWidgetItem(money(line.discountPerUnit)));
        table->setItem((int)i, 6, new QTableWidgetItem(QString::number(line.quantity, 'g', 10)));
        table->setItem((int)i, 7, new QTableWidgetItem(money(lineTotals.totalPayable)));
    }

    //  totals 
    QFormLayout* totals = new QFormLayout();
    totals->addRow("Total without tax:", new QLabel(money(b.totalWithoutTax), this));
    totals->addRow("Total discount:", new QLabel(money(b.totalDiscount), this));
    totals->addRow("Total tax:", new QLabel(money(b.totalTax), this));

    QLabel* payable = new QLabel(money(b.totalPayable), this);
    QFont f = payable->font();
    f.setBold(true);
    payable->setFont(f);
    totals->addRow("Payable:", payable);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addLayout(header);
    layout->addWidget(table);
    layout->addLayout(totals);
    layout->addWidget(buttons);
}