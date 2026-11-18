#include "addproductdialog.h"
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

AddProductDialog::AddProductDialog(IProductRepository& products, QWidget* parent)
    : QDialog(parent), products(products)
{
    setWindowTitle("Add New Product");
    setMinimumWidth(420);

    barcodeEdit = new QLineEdit(this);
    barcodeEdit->setPlaceholderText("6 digits, first digit not 0");
    barcodeEdit->setValidator(new QRegularExpressionValidator(
        QRegularExpression("[1-9][0-9]{0,5}"), this));

    // fill the next free barcode (the manager can still change it)
    products.load();
    barcodeEdit->setText(QString::fromStdString(products.nextBarcode()));

    acronymEdit = new QLineEdit(this);
    acronymEdit->setMaxLength(8);
    acronymEdit->setPlaceholderText("max 8 letters or digits, for example MILK");

    nameEdit = new QLineEdit(this);
    nameEdit->setPlaceholderText("for example Fresh Milk 1L");

    unitBox = new QComboBox(this);
    unitBox->addItems({ "count", "weight" });

    priceBox = new QDoubleSpinBox(this);
    priceBox->setRange(0.0, 10000000.0);
    priceBox->setDecimals(2);

    taxBox = new QDoubleSpinBox(this);
    taxBox->setRange(0.0, 10000000.0);
    taxBox->setDecimals(2);

    discountBox = new QDoubleSpinBox(this);
    discountBox->setRange(0.0, 10000000.0);
    discountBox->setDecimals(2);

    quantityBox = new QDoubleSpinBox(this);
    quantityBox->setRange(0.0, 10000000.0);
    onUnitChanged();   // whole numbers for count, 3 decimals for weight

    QFormLayout* form = new QFormLayout();
    form->addRow("Barcode (6 digits):", barcodeEdit);
    form->addRow("Acronym (max 8):", acronymEdit);
    form->addRow("Product name:", nameEdit);
    form->addRow("Unit:", unitBox);
    form->addRow("Unit price (PKR):", priceBox);
    form->addRow("Tax per unit (PKR):", taxBox);
    form->addRow("Discount per unit (PKR):", discountBox);
    form->addRow("Available quantity:", quantityBox);

    QPushButton* saveButton = new QPushButton("Save Product", this);
    QPushButton* cancelButton = new QPushButton("Cancel", this);
    QHBoxLayout* buttons = new QHBoxLayout();
    buttons->addWidget(saveButton);
    buttons->addWidget(cancelButton);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addLayout(buttons);

    connect(unitBox, &QComboBox::currentTextChanged, this, &AddProductDialog::onUnitChanged);
    connect(saveButton, &QPushButton::clicked, this, &AddProductDialog::onSave);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void AddProductDialog::onUnitChanged()
{
    quantityBox->setDecimals(unitBox->currentText() == "count" ? 0 : 3);
}

void AddProductDialog::onSave()
{
    string barcode = barcodeEdit->text().trimmed().toStdString();
    string acronym = acronymEdit->text().trimmed().toStdString();
    string name = nameEdit->text().trimmed().toStdString();

    if (barcode.size() != 6)
    {
        QMessageBox::warning(this, "Add product", "The barcode must be exactly 6 digits, and cannot start with 0.");
        return;
    }
    if (acronym.empty())
    {
        QMessageBox::warning(this, "Add product", "Enter the acronym.");
        return;
    }
    if (name.empty())
    {
        QMessageBox::warning(this, "Add product", "Enter the product name.");
        return;
    }

    // add() works on the products in memory, so read the file again first
    products.load();

    // id is made by the repository; the path matches where managerwindow saves the image
    Product p("", barcode, acronym, name, unitBox->currentText().toStdString(),
              priceBox->value(), taxBox->value(), discountBox->value(), quantityBox->value(),
              "barcodes/" + barcode + ".png");

    string error;
    if (!products.add(p, error))
    {
        QMessageBox::warning(this, "Cannot add product", QString::fromStdString(error));
        return;   // the form stays open, so the manager can fix it
    }

    products.findByBarcode(barcode, created);   // the saved product, with its new id
    accept();
}