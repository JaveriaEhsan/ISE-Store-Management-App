#include "addproductdialog.h"
#include <string>
#include <vector>
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>
using namespace std;

// The category file stores _ instead of spaces, the screen shows spaces
static string withSpaces(string text)
{
    for (size_t i = 0; i < text.size(); i++)
    {
        if (text[i] == '_')
        {
            text[i] = ' ';
        }
    }
    return text;
}

AddProductDialog::AddProductDialog(IProductRepository& products,
    ICategoryRepository& categories,
    QWidget* parent)
    : QDialog(parent), products(products), categories(categories)
{
    setWindowTitle("Add New Product");
    setMinimumWidth(420);

    // Category list: text "10 - Dairy", the id is kept as item data
    categoryBox = new QComboBox(this);
    vector<Category> all = categories.getAll();
    for (size_t i = 0; i < all.size(); i++)
    {
        QString text = QString::fromStdString(all[i].categoryId + " - " + withSpaces(all[i].name));
        categoryBox->addItem(text, QString::fromStdString(all[i].categoryId));
    }

    // the barcode is made from the category, so it cannot be typed
    barcodeEdit = new QLineEdit(this);
    barcodeEdit->setReadOnly(true);

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
    form->addRow("Category:", categoryBox);
    form->addRow("Barcode (category + serial):", barcodeEdit);
    form->addRow("Acronym (max 8):", acronymEdit);
    form->addRow("Product name:", nameEdit);
    form->addRow("Unit:", unitBox);
    form->addRow("Unit price (PKR):", priceBox);
    form->addRow("Tax per unit (PKR):", taxBox);
    form->addRow("Discount per unit (PKR):", discountBox);
    form->addRow("Available quantity:", quantityBox);

    saveButton = new QPushButton("Save Product", this);
    QPushButton* cancelButton = new QPushButton("Cancel", this);
    QHBoxLayout* buttons = new QHBoxLayout();
    buttons->addWidget(saveButton);
    buttons->addWidget(cancelButton);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addLayout(buttons);

    connect(unitBox, &QComboBox::currentTextChanged, this, &AddProductDialog::onUnitChanged);
    connect(categoryBox, &QComboBox::currentIndexChanged, this, &AddProductDialog::onCategoryChanged);
    connect(saveButton, &QPushButton::clicked, this, &AddProductDialog::onSave);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    // nextBarcode works on the products in memory, so read the file first
    products.load();
    onCategoryChanged();
}

void AddProductDialog::onUnitChanged()
{
    quantityBox->setDecimals(unitBox->currentText() == "count" ? 0 : 3);
}

// Fills the barcode: category id + next 4-digit serial of that category
void AddProductDialog::onCategoryChanged()
{
    if (categoryBox->count() == 0)
    {
        barcodeEdit->setText("");
        barcodeEdit->setPlaceholderText("No categories yet. Add a category first.");
        saveButton->setEnabled(false);
        return;
    }

    string categoryId = categoryBox->currentData().toString().toStdString();
    string barcode = products.nextBarcode(categoryId);

    if (barcode.empty())
    {
        barcodeEdit->setText("");
        barcodeEdit->setPlaceholderText("This category is full (serial 9999 is used).");
        saveButton->setEnabled(false);
        return;
    }

    barcodeEdit->setText(QString::fromStdString(barcode));
    saveButton->setEnabled(true);
}

void AddProductDialog::onSave()
{
    string acronym = acronymEdit->text().trimmed().toStdString();
    string name = nameEdit->text().trimmed().toStdString();

    if (categoryBox->count() == 0)
    {
        QMessageBox::warning(this, "Add product", "Add a category first.");
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

    // add() works on the products in memory, so read the file again first,
    // then take the barcode again in case the file changed meanwhile
    products.load();
    string categoryId = categoryBox->currentData().toString().toStdString();
    string barcode = products.nextBarcode(categoryId);
    if (barcode.empty())
    {
        QMessageBox::warning(this, "Add product", "This category has no free barcode left.");
        return;
    }
    barcodeEdit->setText(QString::fromStdString(barcode));

    // id is made by the repository; the path matches where managerwindow saves the image
    Product p("", barcode, acronym, name, unitBox->currentText().toStdString(),
        priceBox->value(), taxBox->value(), discountBox->value(), quantityBox->value(),
        "barcodes/" + barcode + ".png", categoryId);

    string error;
    if (!products.add(p, error))
    {
        QMessageBox::warning(this, "Cannot add product", QString::fromStdString(error));
        return;   // the form stays open, so the manager can fix it
    }

    products.findByBarcode(barcode, created);   // the saved product, with its new id
    accept();
}