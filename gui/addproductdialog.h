#ifndef ADDPRODUCTDIALOG_H
#define ADDPRODUCTDIALOG_H
#include <QDialog>
#include "../data/IProductRepository.h"
#include "../data/ICategoryRepository.h"
#include "../models/product.h"

class QLineEdit;
class QComboBox;
class QDoubleSpinBox;
class QPushButton;

class AddProductDialog : public QDialog
{
    Q_OBJECT

public:
    AddProductDialog(IProductRepository& products,
        ICategoryRepository& categories,
        QWidget* parent = nullptr);

    // the product that was saved (use it after the dialog is accepted)
    Product createdProduct() const { return created; }

private slots:
    void onSave();
    void onUnitChanged();
    void onCategoryChanged();   // fills the barcode from the chosen category

private:
    IProductRepository& products;
    ICategoryRepository& categories;
    Product created;

    QComboBox* categoryBox;     // shows "10 - Dairy", the id is the item data
    QLineEdit* barcodeEdit;     // read-only, filled by nextBarcode(categoryId)
    QLineEdit* acronymEdit;
    QLineEdit* nameEdit;
    QComboBox* unitBox;
    QDoubleSpinBox* priceBox;
    QDoubleSpinBox* taxBox;
    QDoubleSpinBox* discountBox;
    QDoubleSpinBox* quantityBox;
    QPushButton* saveButton;
};

#endif