#include "inventorywindow.h"
#include "productdialog.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QAbstractItemView>
#include <QStyledItemDelegate>
#include <QDoubleSpinBox>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QTimer>
#include <QFont>
#include <QColor>
#include <algorithm>
#include <cmath>
#include <vector>
#include <string>

using namespace std;

// Columns: 0 ID, 1 Barcode, 2 Acronym, 3 Name, 4 Unit, 5 Price, 6 Tax, 7 Discount, 8 Quantity
static bool isEditableColumn(int col)
{
    return col == 3 || col >= 5;      // Name, Price, Tax, Discount, Quantity
}

// Spin box (with up/down arrows) for Price, Tax, Discount and Quantity.
// Count products accept whole quantities only.
class NumberDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&,
        const QModelIndex& index) const override
    {
        bool wholeNumber = (index.column() == 8 &&
            index.sibling(index.row(), 4).data().toString() == "count");

        QDoubleSpinBox* box = new QDoubleSpinBox(parent);
        box->setDecimals(wholeNumber ? 0 : 2);
        box->setRange(0, 10000000);
        box->setSingleStep(wholeNumber ? 1 : (index.column() == 8 ? 0.5 : 1));
        box->setKeyboardTracking(false);
        return box;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override
    {
        static_cast<QDoubleSpinBox*>(editor)->setValue(index.data().toString().toDouble());
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) const override
    {
        QDoubleSpinBox* box = static_cast<QDoubleSpinBox*>(editor);
        model->setData(index, QString::number(box->value(), 'f', box->decimals()));
    }
};

InventoryWindow::InventoryWindow(IProductRepository* repository,
    ICategoryRepository* categories, QWidget* parent)
    : QDialog(parent), repository(repository), categories(categories)
{
    setWindowTitle("Inventory");
    resize(1000, 600);

    table = new QTableWidget(0, 9);
    QStringList headers;
    headers << "ID" << "Barcode" << "Acronym" << "Name" << "Unit"
        << "Price" << "Tax" << "Discount" << "Quantity";
    table->setHorizontalHeaderLabels(headers);
    table->setEditTriggers(QAbstractItemView::DoubleClicked |
        QAbstractItemView::EditKeyPressed);   // double-click or F2
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSortingEnabled(false);           // sorting would scatter the category rows
    table->verticalHeader()->setVisible(false);

    // Price, Tax, Discount, Quantity use the spin box editor
    NumberDelegate* numbers = new NumberDelegate(table);
    for (int col = 5; col <= 8; col++)
        table->setItemDelegateForColumn(col, numbers);

    QHeaderView* h = table->horizontalHeader();
    h->setSectionResizeMode(QHeaderView::Interactive);
    h->setSectionResizeMode(3, QHeaderView::Stretch);   // Name
    table->setColumnWidth(0, 50);
    table->setColumnWidth(1, 110);
    table->setColumnWidth(2, 110);
    table->setColumnWidth(4, 90);
    table->setColumnWidth(5, 100);
    table->setColumnWidth(6, 90);
    table->setColumnWidth(7, 100);
    table->setColumnWidth(8, 100);

    skippedLabel = new QLabel();
    skippedBox = new QPlainTextEdit();
    skippedBox->setReadOnly(true);
    skippedBox->setMaximumHeight(100);

    QPushButton* refreshButton = new QPushButton("Refresh");
    QPushButton* closeButton = new QPushButton("Close");

    QHBoxLayout* buttons = new QHBoxLayout();
    buttons->addStretch();
    buttons->addWidget(refreshButton);
    buttons->addWidget(closeButton);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Double-click Name, Price, Tax, Discount or Quantity to edit.", this));
    layout->addWidget(table);
    layout->addWidget(skippedLabel);
    layout->addWidget(skippedBox);
    layout->addLayout(buttons);

    connect(refreshButton, &QPushButton::clicked, this, &InventoryWindow::refresh);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);
    connect(table, &QTableWidget::cellChanged, this, &InventoryWindow::onCellChanged);
    // double-click on a read-only cell opens the product details
    connect(table, &QTableWidget::cellDoubleClicked, this,
        [this](int row, int col) { if (!isEditableColumn(col)) showProduct(row); });

    refresh();
}

// Rebuilds the whole table: category header, then its products sorted by barcode
void InventoryWindow::refresh()
{
    refreshing = true;
    {
        QSignalBlocker blocker(table);      // filling must not trigger onCellChanged

        repository->load();
        vector<Product> all = repository->getAll();
        vector<Category> cats = categories->getAll();

        sort(cats.begin(), cats.end(),
            [](const Category& a, const Category& b) { return a.categoryId < b.categoryId; });
        sort(all.begin(), all.end(),
            [](const Product& a, const Product& b) { return a.getBarcode() < b.getBarcode(); });

        table->clearSpans();
        table->setRowCount(0);
        int row = 0;

        // a coloured row that spans every column
        auto addHeader = [&](const QString& text) {
            table->insertRow(row);
            QTableWidgetItem* item = new QTableWidgetItem(text);
            QFont f = item->font();
            f.setBold(true);
            item->setFont(f);
            item->setBackground(QColor("#dbe5f4"));
            item->setFlags(Qt::ItemIsEnabled);          // not selectable, not editable
            table->setItem(row, 0, item);
            table->setSpan(row, 0, 1, table->columnCount());
            row++;
            };

        auto addProduct = [&](const Product& p) {
            table->insertRow(row);

            QStringList cells;
            cells << QString::fromStdString(p.getId())
                << QString::fromStdString(p.getBarcode())
                << QString::fromStdString(p.getAcronym())
                << QString::fromStdString(p.getName())
                << QString::fromStdString(p.getUnit())
                << QString::number(p.getPrice(), 'f', 2)
                << QString::number(p.getTax(), 'f', 2)
                << QString::number(p.getDiscount(), 'f', 2)
                << QString::number(p.getQuantity(), 'g', 10);

            for (int col = 0; col < cells.size(); col++)
            {
                QTableWidgetItem* item = new QTableWidgetItem(cells[col]);
                item->setTextAlignment(Qt::AlignCenter);
                if (!isEditableColumn(col))
                    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                if (col == 8 && p.getQuantity() < 5)     // low stock
                    item->setForeground(QColor("#c62828"));
                table->setItem(row, col, item);
            }
            row++;
            };

        vector<bool> used(all.size(), false);
        for (const Category& c : cats)
        {
            int count = 0;
            for (size_t i = 0; i < all.size(); i++)
                if (all[i].getCategoryId() == c.categoryId) count++;
            if (count == 0) continue;

            QString name = QString::fromStdString(c.name);
            name.replace('_', ' ');
            addHeader(QString::fromStdString(c.categoryId) + " - " + name +
                " (" + QString::fromStdString(c.acronym) + ")   |   " +
                QString::number(count) + " products");

            for (size_t i = 0; i < all.size(); i++)
            {
                if (all[i].getCategoryId() == c.categoryId)
                {
                    addProduct(all[i]);
                    used[i] = true;
                }
            }
        }

        // products whose category is not in PCategory.txt
        bool headerShown = false;
        for (size_t i = 0; i < all.size(); i++)
        {
            if (used[i]) continue;
            if (!headerShown)
            {
                addHeader("Uncategorised");
                headerShown = true;
            }
            addProduct(all[i]);
        }

        // lines that were skipped when the file was read
        vector<string> errors = repository->getErrors();
        QStringList lines;
        for (size_t i = 0; i < errors.size(); i++)
            lines << QString::fromStdString(errors[i]);

        skippedLabel->setText(QString("%1 line(s) were skipped because they could not be read:")
            .arg((int)errors.size()));
        skippedBox->setPlainText(lines.join("\n"));
        skippedLabel->setVisible(!errors.empty());
        skippedBox->setVisible(!errors.empty());
    }
    refreshing = false;
}

// Re-reads one row's editable cells (Name, Price, Tax, Discount, Quantity)
void InventoryWindow::refreshRow(int row)
{
    QTableWidgetItem* barcodeItem = table->item(row, 1);
    if (!barcodeItem) return;                       // category header row

    repository->load();
    Product p;
    if (!repository->findByBarcode(barcodeItem->text().toStdString(), p))
        return;

    refreshing = true;
    {
        QSignalBlocker blocker(table);
        table->item(row, 3)->setText(QString::fromStdString(p.getName()));
        table->item(row, 5)->setText(QString::number(p.getPrice(), 'f', 2));
        table->item(row, 6)->setText(QString::number(p.getTax(), 'f', 2));
        table->item(row, 7)->setText(QString::number(p.getDiscount(), 'f', 2));
        table->item(row, 8)->setText(QString::number(p.getQuantity(), 'g', 10));
        table->item(row, 8)->setForeground(
            p.getQuantity() < 5 ? QBrush(QColor("#c62828")) : QBrush(table->palette().text()));
    }
    refreshing = false;
}

// Called when the user finishes editing a cell: validate, save, show the saved value
void InventoryWindow::onCellChanged(int row, int column)
{
    if (refreshing) return;
    if (!isEditableColumn(column)) return;

    QTableWidgetItem* barcodeItem = table->item(row, 1);
    QTableWidgetItem* edited = table->item(row, column);
    if (!barcodeItem || !edited) return;            // header row

    Product old;
    if (!repository->findByBarcode(barcodeItem->text().toStdString(), old))
        return;

    QString text = edited->text().trimmed();

    // start from the old values and change only the edited one
    string name = old.getName();
    double price = old.getPrice(), tax = old.getTax();
    double discount = old.getDiscount(), quantity = old.getQuantity();
    string error;

    if (column == 3)
    {
        if (text.isEmpty()) error = "The name cannot be empty.";
        else name = text.toStdString();
    }
    else
    {
        bool ok = false;
        double value = text.toDouble(&ok);
        if (!ok || value < 0)
        {
            error = "Enter a valid number (0 or more).";
        }
        else if (column == 5) price = value;
        else if (column == 6) tax = value;
        else if (column == 7) discount = value;
        else if (column == 8)
        {
            if (old.getUnit() == "count" && value != floor(value))
                error = "This product is sold by count. Enter a whole number.";
            else
                quantity = value;
        }
    }

    if (error.empty())
    {
        Product updated(old.getId(), old.getBarcode(), old.getAcronym(), name,
            old.getUnit(), price, tax, discount, quantity,
            old.getBarcodePath(), old.getCategoryId());
        repository->update(updated, error);         // validates with the file rules and saves
    }

    if (!error.empty())
        QMessageBox::warning(this, "Inventory", QString::fromStdString(error));

    // show the saved values (or restore the old ones), after this signal has finished
    QTimer::singleShot(0, this, [this, row]() { refreshRow(row); });
}

void InventoryWindow::showProduct(int row)
{
    QTableWidgetItem* item = table->item(row, 1);   // barcode column
    if (item == nullptr)
    {
        return;
    }

    Product product;
    if (repository->findByBarcode(item->text().toStdString(), product))
    {
        ProductDialog dialog(product, this);
        dialog.exec();
    }
}