#include "billdialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include "../services/BillPdfExporter.h"      
using namespace std;

static QString text(const string& s)
{
    return s.empty() ? QString("-") : QString::fromStdString(s);
}

static QString money(double value)
{
    return QString::number(value, 'f', 2);
}

static QString percentText(double amount, double base)
{
    if (base <= 0 || amount <= 0) return "0 %";

    QString s = QString::number(amount / base * 100.0, 'f', 2);
    while (s.endsWith('0')) s.chop(1);
    if (s.endsWith('.')) s.chop(1);
    return s + " %";
}
// The product name from Product.txt (the file writes spaces as _).
// Without a repository, or if the product is not found, the acronym is shown.
static QString productName(const BillLine& line, const IProductRepository* products)
{
    if (products != nullptr)
    {
        Product p;
        bool found = false;

        if (!line.barcode.empty())
        {
            found = products->findByBarcode(line.barcode, p);
        }
        if (!found)
        {
            found = products->findByAcronym(line.acronym, p);
        }
        if (found)
        {
            return QString::fromStdString(p.getName()).replace('_', ' ');
        }
    }
    return text(line.acronym);
}
BillDialog::BillDialog(const CheckoutResult& result, QWidget* parent,
    const IProductRepository* products)
    : QDialog(parent)
{
    setWindowTitle("Bill");
    resize(620, 600);

    const Order& o = result.order;
    const Bill& b = result.bill;
    shownBill = b;
    // ----- header -----
    QFormLayout* header = new QFormLayout();
    header->addRow("Order id:", new QLabel(text(o.orderId), this));
    header->addRow("Bill id:", new QLabel(text(b.billId), this));
    header->addRow("Date:", new QLabel(text(o.date), this));
    header->addRow("Time:", new QLabel(text(o.time), this));
    header->addRow("Customer:", new QLabel(text(b.customerName), this));
    header->addRow("Payment mode:", new QLabel(text(o.paymentMode), this));
    header->addRow("Merchant:", new QLabel(text(o.merchant), this));
    header->addRow("Status:", new QLabel(text(b.status), this));

    // ----- lines table -----
    // ----- lines table -----
    QTableWidget* table = new QTableWidget((int)b.lines.size(), 8, this);

    table->setHorizontalHeaderLabels(
        { "", "Barcode", "Product", "Qty", "Unit price", "Tax/unit",
          "Discount/unit", "Total" });

    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::NoSelection);

    QHeaderView* tableHeader = table->horizontalHeader();

    tableHeader->setSectionResizeMode(QHeaderView::Fixed);

    table->setColumnWidth(0, 25);
    table->setColumnWidth(1, 62);
    table->setColumnWidth(2, 100);
    table->setColumnWidth(3, 35);
    table->setColumnWidth(4, 65);
    table->setColumnWidth(5, 55);
    table->setColumnWidth(6, 78);
    table->setColumnWidth(7, 60);

    QFont headerFont = tableHeader->font();
    headerFont.setPointSize(8);
    tableHeader->setFont(headerFont);

    table->verticalHeader()->setVisible(false);
    table->setMaximumHeight(220);
    table->setMinimumHeight(120);

    // Calculate seasonal discount percentage
    BillCalculator calculator;

    double discountBase = b.totalWithoutTax - b.totalDiscount;

    double seasonalPercent = 0;

    if (discountBase > 0)
    {
        seasonalPercent = (b.seasonalDiscount / discountBase) * 100.0;
    }

    SeasonalDiscount seasonalRule(seasonalPercent);

    vector<const IDiscountRule*> lineRules;
    lineRules.push_back(&seasonalRule);

    // Fill table
    for (size_t i = 0; i < b.lines.size(); i++)
    {
        const BillLine& line = b.lines[i];

        BillItem item;
        item.unitPrice = line.unitPrice;
        item.discountPerUnit = line.discountPerUnit;
        item.quantity = line.quantity;
        item.taxPerUnit = line.taxPerUnit;

        BillTotals lineTotals =
            calculator.calculate(vector<BillItem>(1, item), lineRules);

        table->setItem((int)i, 0,
            new QTableWidgetItem(QString::number(i + 1)));

        table->setItem((int)i, 1,
            new QTableWidgetItem(text(line.barcode)));

        table->setItem((int)i, 2,
            new QTableWidgetItem(productName(line, products)));

        table->setItem((int)i, 3,
            new QTableWidgetItem(QString::number(line.quantity, 'g', 10)));

        table->setItem((int)i, 4,
            new QTableWidgetItem(money(line.unitPrice)));

        table->setItem((int)i, 5,
            new QTableWidgetItem(money(line.taxPerUnit)));

        table->setItem((int)i, 6,
            new QTableWidgetItem(money(line.discountPerUnit)));

        table->setItem((int)i, 7,
            new QTableWidgetItem(money(lineTotals.totalPayable)));
    }

    // ----- totals -----
    QFormLayout* totals = new QFormLayout();
    totals->addRow("Total without tax:", new QLabel(money(b.totalWithoutTax), this));
    totals->addRow("Product discounts:", new QLabel(money(b.totalDiscount), this));

    totals->addRow("Seasonal discount (" + percentText(b.seasonalDiscount, discountBase) + "):",
        new QLabel(money(b.seasonalDiscount), this));
    totals->addRow("Special discount (" + percentText(b.specialDiscount, discountBase) + "):",
        new QLabel(money(b.specialDiscount), this));
    totals->addRow("Total tax:", new QLabel(money(b.totalTax), this));

    QLabel* payable = new QLabel(money(b.totalPayable), this);
    QFont f = payable->font();
    f.setBold(true);
    payable->setFont(f);
    totals->addRow("Payable:", payable);

    totals->addRow("Amount received:", new QLabel(money(b.amountReceived), this));
    totals->addRow("Balance returned:", new QLabel(money(b.balanceReturned), this));
    if (result.editedPaidOrder)
    {
        QString changeText;
        if (result.editDifference > 0)       changeText = "Collected " + money(result.editDifference);
        else if (result.editDifference < 0)  changeText = "Refunded " + money(qAbs(result.editDifference));
        else                                 changeText = "Nothing to pay or refund";

        totals->addRow("Change from old bill:", new QLabel(changeText, this));
    }
    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    QPushButton* pdfButton = buttons->addButton("Download PDF", QDialogButtonBox::ActionRole);
    connect(pdfButton, &QPushButton::clicked, this, &BillDialog::savePdf);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    noteLabel = new QLabel(this);
    QFont noteFont = noteLabel->font();
    noteFont.setBold(true);
    noteLabel->setFont(noteFont);
    noteLabel->hide();

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(noteLabel);
    layout->addLayout(header);
    layout->addWidget(table);
    layout->addLayout(totals);
    layout->addWidget(buttons);

    // a bill made by editing a paid order says which bill it replaces
    if (!result.replacedBillId.empty())
    {
        setNote("REVISED - replaces bill " + QString::fromStdString(result.replacedBillId));
    }
}

void BillDialog::setNote(const QString& note)
{
    noteLabel->setText(note);
    noteLabel->setVisible(!note.isEmpty());
}
void BillDialog::setOrderBills(const vector<Bill>& bills)
{
    orderBills = bills;
}

void BillDialog::savePdf()
{
    QString path = QFileDialog::getSaveFileName(
        this, "Save bill as PDF",
        "Bill_" + QString::fromStdString(shownBill.billId) + ".pdf",
        "PDF files (*.pdf)");
    if (path.isEmpty())
    {
        return;
    }
    if (!path.endsWith(".pdf", Qt::CaseInsensitive))
    {
        path += ".pdf";
    }

    string note;
    const Bill* previous = nullptr;
    if (!orderBills.empty())
    {
        note = BillPdfExporter::makeNote(shownBill, orderBills);
        previous = BillPdfExporter::previousBill(shownBill, orderBills);
    }
    else
    {
        note = noteLabel->text().toStdString();
    }

    BillPdfExporter exporter;
    if (exporter.exportBill(shownBill, path.toStdString(), note, previous))
    {
        QMessageBox::information(this, "PDF saved", "The bill was saved to:\n" + path);
    }
    else
    {
        QMessageBox::warning(this, "PDF failed", "The PDF could not be created. Check the folder and try again.");
    }
}