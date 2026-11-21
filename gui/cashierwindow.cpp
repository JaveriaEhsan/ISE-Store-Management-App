#include "cashierwindow.h"
#include "paymentdialog.h"
#include "billdialog.h"
#include "scandialog.h"
#include "productdialog.h"
#include "heldordersdialog.h"
#include "models/order.h"
#include <algorithm>
#include <QLineEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QEvent>
#include <QMouseEvent>
#include <QFont>
#include <QSignalBlocker>
#include <QDoubleSpinBox>

using namespace std;

// A table cell the user cannot edit
static QTableWidgetItem* readOnly(const QString& text)
{
    QTableWidgetItem* item = new QTableWidgetItem(text);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    return item;
}

static QString buttonStyle()
{
    return "QPushButton { background-color: #1f3a5f; color: white }";
}

CashierWindow::CashierWindow(IProductRepository& products, OrderRepository& orders,
                             OrderDetailRepository& orderDetails, BillRepository& bills,
                             QWidget* parent)
    : QWidget(parent), products(products), orders(orders),
      orderDetails(orderDetails), bills(bills)
{
    setWindowTitle("SmartPOS - Cashier");
    resize(950, 620);

    // ---------- top bar ----------
    QWidget* header = new QWidget(this);
    header->setObjectName("header");
    header->setFixedHeight(100);
    header->setStyleSheet("QWidget#header { background-color: #1f3a5f; }");

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
    QPushButton* resumeButton = new QPushButton("Resume Held Order", content);
    addButton->setStyleSheet(buttonStyle());
    descButton->setStyleSheet(buttonStyle());
    removeButton->setStyleSheet(buttonStyle());
    resumeButton->setStyleSheet(buttonStyle());

    QHBoxLayout* itemButtons = new QHBoxLayout();
    itemButtons->addWidget(addButton);
    itemButtons->addWidget(descButton);
    itemButtons->addWidget(removeButton);
    itemButtons->addWidget(resumeButton);

    table = new QTableWidget(0, 7, content);
    table->setHorizontalHeaderLabels({ "Barcode", "Product", "Quantity",
                                       "Unit price", "Tax / unit", "Discount / unit",
                                       "Total" });
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->verticalHeader()->setDefaultSectionSize(36);   // space for the spin box
    table->viewport()->installEventFilter(this);

    totalsLabel = new QLabel(content);
    totalsLabel->setTextFormat(Qt::RichText);

    QPushButton* billButton = new QPushButton("Generate Bill", content);
    QPushButton* holdButton = new QPushButton("Hold Order", content);
    QPushButton* cancelButton = new QPushButton("Cancel Order", content);
    QHBoxLayout* bottom = new QHBoxLayout();
    bottom->addWidget(billButton);
    bottom->addWidget(holdButton);
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
    connect(resumeButton, &QPushButton::clicked, this, &CashierWindow::onResumeOrder);
    connect(billButton, &QPushButton::clicked, this, &CashierWindow::onGenerateBill);
    connect(holdButton, &QPushButton::clicked, this, &CashierWindow::onHoldOrder);
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
    {
        QSignalBlocker blocker(table);      // filling the table must not trigger onCellChanged

        const vector<BillItem>& items = service.getItems();
        table->setRowCount((int)items.size());

        QFont bold;
        bold.setBold(true);

        for (int row = 0; row < (int)items.size(); row++)
        {
            const BillItem& it = items[row];
            double lineTotal = (it.unitPrice - it.discountPerUnit + it.taxPerUnit) * it.quantity;

            table->setItem(row, 0, readOnly(QString::fromStdString(it.barcode)));
            table->setItem(row, 1, readOnly(QString::fromStdString(it.acronym)));
            table->setItem(row, 2, readOnly(""));          // the spin box sits on top of this cell
            table->setItem(row, 3, readOnly(QString::number(it.unitPrice, 'f', 2)));
            table->setItem(row, 4, readOnly(QString::number(it.taxPerUnit, 'f', 2)));
            table->setItem(row, 5, readOnly(QString::number(it.discountPerUnit, 'f', 2)));

            QTableWidgetItem* totalItem = readOnly(QString::number(lineTotal, 'f', 2));
            totalItem->setFont(bold);
            table->setItem(row, 6, totalItem);

            // quantity spin box
            Product p;
            bool isCount = false;
            double maxQty = 100000;
            if (products.findByBarcode(it.barcode, p))
            {
                isCount = (p.getUnit() == "count");
                maxQty = p.getQuantity();
            }
            double minQty = isCount ? 1.0 : 0.01;

            QDoubleSpinBox* spin = new QDoubleSpinBox(table);
            spin->setDecimals(isCount ? 0 : 2);            // count: whole numbers only
            spin->setRange(minQty, std::max(maxQty, minQty));
            spin->setSingleStep(isCount ? 1.0 : 0.5);
            spin->setKeyboardTracking(false);              // typed value applies on Enter
            spin->setValue(it.quantity);

            string barcode = it.barcode;
            connect(spin, &QDoubleSpinBox::valueChanged, this,
                    [this, barcode, spin](double value) { onQuantitySpin(barcode, value, spin); });

            table->setCellWidget(row, 2, spin);
        }

        table->clearSelection();
        table->setCurrentItem(nullptr);
    }
    showTotals();
}

void CashierWindow::showTotals()
{
    BillTotals t = service.getTotals();   // all arithmetic stays in BillCalculator
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
bool CashierWindow::isValidContact(const QString& customerContact) const
{
    if (customerContact.isEmpty()) return true;       // optional
    if (customerContact.size() != 11) return false;
    for (QChar c : customerContact)
    {
        if (!c.isDigit()) return false;
    }
    return true;
}

// Checks the name and contact boxes. Shows a message and returns false if invalid.
bool CashierWindow::readCustomer(string& name, string& contact)
{
    QString nameText = nameEdit->text().trimmed();
    if (nameText.isEmpty())
    {
        QMessageBox::warning(this, "Customer", "Enter the customer name.");
        nameEdit->setFocus();
        return false;
    }
    if (!isValidName(nameText))
    {
        QMessageBox::warning(this, "Invalid name",
            "The customer name can contain only letters and spaces.\n"
            "Numbers and symbols are not allowed.");
        nameEdit->setFocus();
        return false;
    }

    QString contactText = contactEdit->text().trimmed();
    if (!isValidContact(contactText))
    {
        QMessageBox::warning(this, "Invalid contact",
            "The contact number must be exactly 11 digits, for example 03001234567.\n"
            "Leave it empty if the customer has no number.");
        contactEdit->setFocus();
        return false;
    }

    name = nameText.toStdString();
    contact = contactText.toStdString();
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

    string error;
    if (!service.addItem(p, 1, error))     // quantity 1, edit it in the table
    {
        QMessageBox::warning(this, "Cannot add", QString::fromStdString(error));
        return;
    }
    refreshTable();

    // select the row of this product, so the cashier can edit it right away
    const vector<BillItem>& items = service.getItems();
    for (size_t i = 0; i < items.size(); i++)
    {
        if (items[i].barcode == barcode)
        {
            table->selectRow((int)i);
            table->scrollToItem(table->item((int)i, 0));
            break;
        }
    }
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

void CashierWindow::onQuantitySpin(const string& barcode, double value, QDoubleSpinBox* spin)
{
    const vector<BillItem>& items = service.getItems();
    int row = -1;
    for (int i = 0; i < (int)items.size(); i++)
    {
        if (items[i].barcode == barcode) { row = i; break; }
    }
    if (row < 0) return;

    double oldQuantity = items[row].quantity;

    Product p;
    string error;
    if (!products.findByBarcode(barcode, p))
        error = "This product is no longer in the inventory.";
    else
        service.setQuantity(p, value, error);

    if (!error.empty())
    {
        QMessageBox::warning(this, "Quantity", QString::fromStdString(error));
        QSignalBlocker blocker(spin);
        spin->setValue(oldQuantity);                
        return;
    }

    // accepted: update this row's total and the order totals
    const BillItem& it = service.getItems()[row];
    double lineTotal = (it.unitPrice - it.discountPerUnit + it.taxPerUnit) * it.quantity;
    if (table->item(row, 6))
        table->item(row, 6)->setText(QString::number(lineTotal, 'f', 2));
    table->selectRow(row);                         
    showTotals();
}

void CashierWindow::onGenerateBill()
{
    if (service.getItems().empty())
    {
        QMessageBox::warning(this, "Generate bill", "The order is empty.");
        return;
    }

    string name, contact;
    if (!readCustomer(name, contact)) return;

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

// Saves the current order as "on-hold" and starts a new, empty order
void CashierWindow::onHoldOrder()
{
    if (service.getItems().empty())
    {
        QMessageBox::warning(this, "Hold order", "The order is empty.");
        return;
    }

    string name, contact;
    if (!readCustomer(name, contact)) return;

    string orderId, error;
    if (!service.holdOrder(name, contact, orders, orderDetails, orderId, error))
    {
        QMessageBox::critical(this, "Could not hold the order", QString::fromStdString(error));
        return;
    }

    QMessageBox::information(this, "Order on hold",
        "The order was put on hold.\nOrder ID: " + QString::fromStdString(orderId) +
        "\n\nUse \"Resume Held Order\" to continue it later.");
    resetScreen();
}

// Lists the held orders and loads the chosen one back into the screen
void CashierWindow::onResumeOrder()
{
    if (!service.getItems().empty())
    {
        QMessageBox::warning(this, "Resume order",
            "Finish, hold or cancel the current order first.");
        return;
    }

    vector<Order> held = orders.findByStatus(STATUS_ON_HOLD);
    if (held.empty())
    {
        QMessageBox::information(this, "Resume order", "There are no held orders.");
        return;
    }

    HeldOrdersDialog dialog(held, this);
    if (dialog.exec() != QDialog::Accepted) return;
    Order chosen = dialog.selectedOrder();

    string error;
    bool loaded = service.resumeOrder(chosen.orderId, products, orderDetails, error);
    if (!loaded)
    {
        QMessageBox::warning(this, "Resume order",
            "The order could not be loaded.\n" + QString::fromStdString(error));
        return;
    }
    if (!error.empty())     // some items could not be restored (for example out of stock)
    {
        QMessageBox::warning(this, "Resume order",
            "Some items could not be restored:\n" + QString::fromStdString(error));
    }

    QString name = QString::fromStdString(chosen.customerName);
    name.replace('_', ' ');                       // the file stores spaces as _
    nameEdit->setText(name);
    contactEdit->setText(chosen.customerContact == "-" ? QString() : QString::fromStdString(chosen.customerContact));
    refreshTable();
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