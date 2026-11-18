#ifndef ADDPRODUCTDIALOG_H
#define ADDPRODUCTDIALOG_H

#include <QDialog>
#include "../data/IProductRepository.h"
#include "../models/product.h"

class QLineEdit;
class QComboBox;
class QDoubleSpinBox;

class AddProductDialog : public QDialog
{
    Q_OBJECT

public:
    AddProductDialog(IProductRepository& products, QWidget* parent = nullptr);

    // the product that was saved (use it after the dialog is accepted)
    Product createdProduct() const { return created; }

private slots:
    void onSave();
    void onUnitChanged();

private:
    IProductRepository& products;
    Product created;

    QLineEdit* barcodeEdit;
    QLineEdit* acronymEdit;
    QLineEdit* nameEdit;
    QComboBox* unitBox;
    QDoubleSpinBox* priceBox;
    QDoubleSpinBox* taxBox;
    QDoubleSpinBox* discountBox;
    QDoubleSpinBox* quantityBox;
};

#endif