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

using namespace std;

InventoryWindow::InventoryWindow(IProductRepository* repository, QWidget* parent)
    : QDialog(parent)
{
    this->repository = repository;
    setWindowTitle("Inventory");
    resize(1000, 550);

    table = new QTableWidget(0, 9);
    QStringList headers;
    headers << "ID" << "Barcode" << "Acronym" << "Name" << "Unit"
        << "Price" << "Tax" << "Discount" << "Quantity";
    table->setHorizontalHeaderLabels(headers);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->verticalHeader()->setVisible(false);
    table->setAlternatingRowColors(true);

    // column widths: ID small, Name stretch, remaining wide
    QHeaderView* h = table->horizontalHeader();
    h->setSectionResizeMode(QHeaderView::Interactive);
    h->setSectionResizeMode(3, QHeaderView::Stretch);   // Name
    table->setColumnWidth(0, 50);    // ID
    table->setColumnWidth(1, 110);   // Barcode
    table->setColumnWidth(2, 110);   // Acronym
    table->setColumnWidth(4, 90);    // Unit
    table->setColumnWidth(5, 100);   // Price
    table->setColumnWidth(6, 90);    // Tax
    table->setColumnWidth(7, 100);   // Discount
    table->setColumnWidth(8, 100);   // Quantity

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
    layout->addWidget(table);
    layout->addWidget(skippedLabel);
    layout->addWidget(skippedBox);
    layout->addLayout(buttons);

    connect(refreshButton, &QPushButton::clicked, this, &InventoryWindow::refresh);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);
    connect(table, &QTableWidget::cellDoubleClicked, this,
        [this](int row, int) { showProduct(row); });

    refresh();
}

void InventoryWindow::refresh()
{
    repository->load();  
    vector<Product> products = repository->getAll();
    table->setRowCount((int)products.size());

    for (int row = 0; row < (int)products.size(); row++)
    {
        const Product& p = products[row];

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
            table->setItem(row, col, item);
        }
    }

    // lines that were skipped when the file was read
    vector<string> errors = repository->getErrors();
    QStringList lines;
    for (size_t i = 0; i < errors.size(); i++)
    {
        lines << QString::fromStdString(errors[i]);
    }
    skippedLabel->setText(QString("%1 line(s) were skipped because they could not be read:")
        .arg((int)errors.size()));
    skippedBox->setPlainText(lines.join("\n"));
    skippedLabel->setVisible(!errors.empty());
    skippedBox->setVisible(!errors.empty());
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