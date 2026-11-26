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
#include <QDialogButtonBox>
#include <QLabel>
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
    newCategoryButton = new QPushButton("New category...", this);
    loadCategories("");

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
    QHBoxLayout* categoryRow = new QHBoxLayout();
    categoryRow->addWidget(categoryBox, 1);
    categoryRow->addWidget(newCategoryButton);
    form->addRow("Category:", categoryRow);
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
    connect(newCategoryButton, &QPushButton::clicked, this, &AddProductDialog::onNewCategory);
    connect(saveButton, &QPushButton::clicked, this, &AddProductDialog::onSave);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    // nextBarcode works on the products in memory, so read the file first
    products.load();
    onCategoryChanged();
}

// Fills the category list from the repository. If selectId is given, that category is selected.
void AddProductDialog::loadCategories(const string& selectId)
{
    categoryBox->blockSignals(true);   // no barcode update while the list is being filled
    categoryBox->clear();

    vector<Category> all = categories.getAll();
    int selectRow = 0;
    for (size_t i = 0; i < all.size(); i++)
    {
        QString text = QString::fromStdString(all[i].categoryId + " - " + withSpaces(all[i].name));
        categoryBox->addItem(text, QString::fromStdString(all[i].categoryId));
        if (all[i].categoryId == selectId)
        {
            selectRow = (int)i;
        }
    }
    if (categoryBox->count() > 0)
    {
        categoryBox->setCurrentIndex(selectRow);
    }
    categoryBox->blockSignals(false);
}

// Asks for a name and an acronym, gives the category the next free id, then selects it
void AddProductDialog::onNewCategory()
{
    string id = categories.nextCategoryId();
    if (id.empty())
    {
        QMessageBox::warning(this, "New category", "All category ids (10 to 99) are already used.");
        return;
    }

    QDialog form(this);
    form.setWindowTitle("New category");

    QLineEdit* catName = new QLineEdit(&form);
    catName->setPlaceholderText("for example Frozen Food");
    QLineEdit* catAcronym = new QLineEdit(&form);
    catAcronym->setPlaceholderText("for example FRZ");

    QFormLayout* fields = new QFormLayout();
    fields->addRow("Id (automatic):", new QLabel(QString::fromStdString(id), &form));
    fields->addRow("Name:", catName);
    fields->addRow("Acronym:", catAcronym);

    QDialogButtonBox* box = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &form);
    connect(box, &QDialogButtonBox::accepted, &form, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, &form, &QDialog::reject);

    QVBoxLayout* layout = new QVBoxLayout(&form);
    layout->addLayout(fields);
    layout->addWidget(box);

    if (form.exec() != QDialog::Accepted)
    {
        return;
    }

    Category category;
    category.categoryId = id;
    category.name = catName->text().trimmed().toStdString();
    category.acronym = catAcronym->text().trimmed().toStdString();

    string error;
    if (!categories.add(category, error))
    {
        QMessageBox::warning(this, "Cannot add category", QString::fromStdString(error));
        return;
    }

    // show the new category and make its barcode
    loadCategories(id);
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