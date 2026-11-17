#include "productdialog.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

ProductDialog::ProductDialog(const Product& product, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Product Details");

    QFormLayout* form = new QFormLayout();
    form->addRow("ID:", new QLabel(QString::fromStdString(product.getId())));
    form->addRow("Barcode:", new QLabel(QString::fromStdString(product.getBarcode())));
    form->addRow("Acronym:", new QLabel(QString::fromStdString(product.getAcronym())));
    form->addRow("Name:", new QLabel(QString::fromStdString(product.getName())));
    form->addRow("Unit:", new QLabel(QString::fromStdString(product.getUnit())));
    form->addRow("Price (PKR):", new QLabel(QString::number(product.getPrice(), 'f', 2)));
    form->addRow("Tax (PKR):", new QLabel(QString::number(product.getTax(), 'f', 2)));
    form->addRow("Discount (PKR):", new QLabel(QString::number(product.getDiscount(), 'f', 2)));
    form->addRow("Available quantity:", new QLabel(QString::number(product.getQuantity(), 'g', 10)));
    form->addRow("Barcode image:", new QLabel(QString::fromStdString(product.getBarcodePath())));

    QPushButton* closeButton = new QPushButton("Close");
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(closeButton);
}