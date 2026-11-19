#include "cashierwindow.h"
#include "paymentdialog.h"
#include "billdialog.h"
#include "scandialog.h"
#include "productdialog.h"   
#include <QLineEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QInputDialog>
#include <QEvent>
#include <QMouseEvent>
#include <QFont>

using namespace std;

CashierWindow::CashierWindow(IProductRepository& products, OrderRepository& orders,
                             OrderDetailRepository& orderDetails, BillRepository& bills,
                             QWidget* parent)
    : QWidget(parent), products(products), orders(orders),
      orderDetails(orderDetails), bills(bills)
{
    setWindowTitle("SmartPOS - Cashier");
    resize(900, 620);

    // ---------- top bar (same look as the manager window) ----------
    QWidget* header = new QWidget(this);
    header->setObjectName("header");
    header->setFixedHeight(100);
    header->setStyleSheet("QWidget#header { background-color: #07143E; }");

    QVBoxLayout* headerLayout = new QVBoxLayout(header);
    headerLayout->setContentsMargins(28, 14, 28, 12);
    headerLayout->setSpacing(6);

    QLabel* logo = new QLabel("SmartPOS", header);
    logo->setStyleSheet("color: white; font-size: 30px; font-weight: bold;");

    QLabel* badge = new QLabel("Cashier", header);
    badge->setAlignment(Qt::AlignCenter);
    badge->setFixedSize(100, 34);
    badge->setStyleSheet(
        "background-color: #15952C; color: white; font-size: 15px;"
        "font-weight: bold; border-radius: 17px;");

    QPushButton* backButton = new QPushButton("< Back", header);
    backButton->setFixedSize(90, 34);
    backButton->setCursor(Qt::PointingHandCursor);
    backButton->setStyleSheet(
        "QPushButton { background-color: transparent; color: white;"
        "font-size: 15px; font-weight: bold; border: 2px solid white;"
        "border-radius: 17px; }"
        "QPushButton:hover { background-color: #1B3485; }");

    QHBoxLayout* brandRow = new QHBoxLayout();
    brandRow->setSpacing(12);
    brandRow->addWidget(logo);
    brandRow->addStretch(1);
    brandRow->addWidget(backButton);
    brandRow->addSpacing(10);
    brandRow->addWidget(badge);

    QLabel* subtitle = new QLabel("New Order", header);
    subtitle->setStyleSheet("color: white; font-size: 17px;");

    headerLayout->addLayout(brandRow);
    headerLayout->addWidget(subtitle);

    // ---------- content below the top bar ----------
    QWidget* content = new QWidget(this);
    content->setObjectName("content");
    content->setStyleSheet("QWidget#content { background-color: #F4F6FA; }");

    nameEdit = new QLineEdit(content);
    nameEdit->setPlaceholderText("letters and spaces only");
    contactEdit = new QLineEdit(content);
    contactEdit->setPlaceholderText("optional, 11 digits (03001234567)");
    contactEdit->setMaxLength(11);

    QFormLayout* form = new QFormLayout();
    form->addRow("Customer name:", nameEdit);
    form->addRow("Customer contact:", contactEdit);

    QPushButton* addButton = new QPushButton("Add Product to Bill", content);
    QPushButton* descButton = new QPushButton("Description", content);
    QPushButton* removeButton = new QPushButton("Remove Item", content);
    QHBoxLayout* itemButtons = new QHBoxLayout();
    itemButtons->addWidget(addButton);
    itemButtons->addWidget(descButton);
    itemButtons->addWidget(removeButton);

    table = new QTableWidget(0, 7, content);
    table->setHorizontalHeaderLabels({ "Barcode", "Product", "Quantity",
                                       "Unit price", "Tax / unit", "Discount / unit",
                                       "Total" });
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->viewport()->installEventFilter(this);

    totalsLabel = new QLabel(content);
    totalsLabel->setTextFormat(Qt::RichText);

    QPushButton* billButton = new QPushButton("Generate Bill", content);
    QPushButton* cancelButton = new QPushButton("Cancel Order", content);
    QHBoxLayout* bottom = new QHBoxLayout();
    bottom->addWidget(billButton);
    bottom->addWidget(cancelButton);

    QVBoxLayout* contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(20, 16, 20, 16);
    contentLayout->addLayout(form);
    contentLayout->addLayout(itemButtons);
    contentLayout->addWidget(table);
    contentLayout->addWidget(totalsLabel);
    contentLayout->addLayout(bottom);

    // ---------- whole window: header on top, content below ----------
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    root->addWidget(header);
    root->addWidget(content, 1);

    connect(backButton, &QPushButton::clicked, this, &CashierWindow::onBack);
    connect(addButton, &QPushButton::clicked, this, &CashierWindow::onAddProduct);
    connect(descButton, &QPushButton::clicked, this, &CashierWindow::onDescription);
    connect(removeButton, &QPushButton::clicked, this, &CashierWindow::onRemoveItem);
    connect(billButton, &QPushButton::clicked, this, &CashierWindow::onGenerateBill);
    connect(cancelButton, &QPushButton::clicked, this, &CashierWindow::onCancelOrder);

    refreshTable();
}

// Clicking an empty area of the table clears the selection
bool CashierWindow::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == table->viewport() && event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        if (!table->indexAt(me->pos()).isValid())
        {
            table->clearSelection();
            table->setCurrentItem(nullptr);
        }
    }
    return QWidget::eventFilter(watched, event);
}

// ---------- showing the order ----------

void CashierWindow::refreshTable()
{
    const vector<BillItem>& items = service.getItems();
    table->setRowCount((int)items.size());

    QFont bold;
    bold.setBold(true);

    for (int row = 0; row < (int)items.size(); row++)
    {
        const BillItem& it = items[row];
        double lineTotal = (it.unitPrice - it.discountPerUnit + it.taxPerUnit) * it.quantity;

        table->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(it.barcode)));
        table->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(it.acronym)));
        table->setItem(row, 2, new QTableWidgetItem(QString::number(it.quantity)));
        table->setItem(row, 3, new QTableWidgetItem(QString::number(it.unitPrice, 'f', 2)));
        table->setItem(row, 4, new QTableWidgetItem(QString::number(it.taxPerUnit, 'f', 2)));
        table->setItem(row, 5, new QTableWidgetItem(QString::number(it.discountPerUnit, 'f', 2)));

        QTableWidgetItem* totalItem = new QTableWidgetItem(QString::number(lineTotal, 'f', 2));
        totalItem->setFont(bold);                       // bold line total
        table->setItem(row, 6, totalItem);
    }

    table->clearSelection();
    table->setCurrentItem(nullptr);

    BillTotals t = service.getTotals();
    totalsLabel->setText(
        QString("Without tax: %1 &nbsp;&nbsp; Discount: %2 &nbsp;&nbsp; Tax: %3<br>"
                "<span style='font-size:22px; font-weight:bold; color:#1f3a5f;'>"
                "PAYABLE: %4 PKR</span>")
            .arg(t.totalWithoutTax, 0, 'f', 2)
            .arg(t.totalDiscount, 0, 'f', 2)
            .arg(t.totalTax, 0, 'f', 2)
            .arg(t.totalPayable, 0, 'f', 2));
}

void CashierWindow::resetScreen()
{
    service.clear();
    nameEdit->clear();
    contactEdit->clear();
    refreshTable();
}

void CashierWindow::showNotFound(const string& barcode)
{
    QMessageBox::warning(this, "Product not found",
        "No product exists against the barcode " + QString::fromStdString(barcode) +
        ".\n\nCheck the barcode and try again.");
}

// A name may contain only letters and spaces, and at least one letter
bool CashierWindow::isValidName(const QString& name) const
{
    bool hasLetter = false;
    for (QChar c : name)
    {
        if (c.isLetter()) hasLetter = true;
        else if (!c.isSpace()) return false;
    }
    return hasLetter;
}

// Contact is optional. If given, it must be exactly 11 digits (for example 03001234567)
bool CashierWindow::isValidContact(const QString& contact) const
{
    if (contact.isEmpty()) return true;       // optional
    if (contact.size() != 11) return false;
    for (QChar c : contact)
    {
        if (!c.isDigit()) return false;
    }
    return true;
}

// ---------- buttons ----------

void CashierWindow::onBack()
{
    if (!service.getItems().empty())
    {
        if (QMessageBox::question(this, "Go back",
                "This order has items. Going back will discard it. Continue?")
            != QMessageBox::Yes)
        {
            return;
        }
    }
    close();       // the main window reappears (see mainwindow.cpp)
}

void CashierWindow::onAddProduct()
{
    string barcode;
    if (!askBarcode(barcode)) return;

    Product p;
    if (!products.findByBarcode(barcode, p))
    {
        showNotFound(barcode);
        return;
    }

    bool ok = false;
    double quantity = 1;
    QString label = "Quantity of " + QString::fromStdString(p.getName()) + ":";
    if (p.getUnit() == "count")
        quantity = QInputDialog::getInt(this, "Quantity", label, 1, 1, 100000, 1, &ok);
    else
        quantity = QInputDialog::getDouble(this, "Quantity", label, 1.0, 0.001, 100000, 3, &ok);
    if (!ok) return;

    string error;
    if (!service.addItem(p, quantity, error))
    {
        QMessageBox::warning(this, "Cannot add", QString::fromStdString(error));
        return;
    }
    refreshTable();
}

void CashierWindow::onDescription()
{
    const vector<BillItem>& items = service.getItems();

    string barcode;
    if (!table->selectedItems().isEmpty())
    {
        int row = table->currentRow();
        if (row < 0 || row >= (int)items.size()) return;
        barcode = items[row].barcode;
    }
    else if (!askBarcode(barcode))
    {
        return;
    }

    Product p;
    if (!products.findByBarcode(barcode, p))
    {
        showNotFound(barcode);
        return;
    }
    showProduct(p);
}

void CashierWindow::onRemoveItem()
{
    const vector<BillItem>& items = service.getItems();
    int row = table->currentRow();
    if (table->selectedItems().isEmpty() || row < 0 || row >= (int)items.size())
    {
        QMessageBox::information(this, "Remove item", "Select an item in the table first.");
        return;
    }
    string barcode = items[row].barcode;
    service.removeItem(barcode);
    refreshTable();
}

void CashierWindow::onGenerateBill()
{
    if (service.getItems().empty())
    {
        QMessageBox::warning(this, "Generate bill", "The order is empty.");
        return;
    }

    QString nameText = nameEdit->text().trimmed();
    if (nameText.isEmpty())
    {
        QMessageBox::warning(this, "Generate bill", "Enter the customer name.");
        return;
    }
    if (!isValidName(nameText))
    {
        QMessageBox::warning(this, "Invalid name",
            "The customer name can contain only letters and spaces.\n"
            "Numbers and symbols are not allowed.");
        nameEdit->setFocus();
        return;
    }
    QString contactText = contactEdit->text().trimmed();
    if (!isValidContact(contactText))
    {
        QMessageBox::warning(this, "Invalid contact",
            "The contact number must be exactly 11 digits, for example 03001234567.\n"
            "Leave it empty if the customer has no number.");
        contactEdit->setFocus();
        return;
    }

    string name = nameText.toStdString();
    string contact = contactText.toStdString();
    PaymentDialog paymentDialog(name, contact, this);
    if (paymentDialog.exec() != QDialog::Accepted) return;
    CheckoutInfo info = paymentDialog.info();

    CheckoutResult result;
    string error;
    if (!service.checkout(info, products, orders, orderDetails, bills, result, error))
    {
        QMessageBox::critical(this, "Could not generate the bill",
                              QString::fromStdString(error));
        refreshTable();
        return;
    }

    BillDialog billDialog(result, this);
    billDialog.exec();
    resetScreen();
}

void CashierWindow::onCancelOrder()
{
    if (service.getItems().empty()) { resetScreen(); return; }
    if (QMessageBox::question(this, "Cancel order", "Remove all items from this order?")
        == QMessageBox::Yes)
        resetScreen();
}

// ---------- dialogs ----------

bool CashierWindow::askBarcode(string& barcode)
{
    ScanDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) return false;
    barcode = dialog.barcode().toStdString();
    return true;
}

void CashierWindow::showProduct(const Product& p)
{
    ProductDialog dialog(p, this); 
    dialog.exec();
}