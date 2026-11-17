#ifndef INVENTORYWINDOW_H
#define INVENTORYWINDOW_H

#include <QDialog>
#include "../data/IProductRepository.h"

class QTableWidget;
class QLabel;
class QPlainTextEdit;

// Table of all products, plus the lines that could not be read from the file
class InventoryWindow : public QDialog
{
public:
    InventoryWindow(IProductRepository* repository, QWidget* parent = nullptr);

    // Reads the products again and refills the table
    void refresh();

private:
    IProductRepository* repository;
    QTableWidget* table;
    QLabel* skippedLabel;
    QPlainTextEdit* skippedBox;

    // Opens the product dialog for the double-clicked row
    void showProduct(int row);
};

#endif
