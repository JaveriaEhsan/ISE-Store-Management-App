#ifndef PRODUCTDIALOG_H
#define PRODUCTDIALOG_H

#include <QDialog>
#include "../models/product.h"

// Shows all fields of one product
class ProductDialog : public QDialog
{
public:
    ProductDialog(const Product& product, QWidget* parent = nullptr);
};

#endif
