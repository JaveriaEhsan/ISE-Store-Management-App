#ifndef INVENTORYWINDOW_H
#define INVENTORYWINDOW_H

#include <QDialog>
#include "data/IProductRepository.h"
#include "data/ICategoryRepository.h"

class QTableWidget;
class QLabel;
class QPlainTextEdit;

class InventoryWindow : public QDialog
{
    Q_OBJECT

public:
    InventoryWindow(IProductRepository* repository, ICategoryRepository* categories,
        QWidget* parent = nullptr);

private slots:
    void refresh();
    void onCellChanged(int row, int column);

private:
    void refreshRow(int row);
    void showProduct(int row);

    IProductRepository* repository;
    ICategoryRepository* categories;
    QTableWidget* table;
    QLabel* skippedLabel;
    QPlainTextEdit* skippedBox;
    bool refreshing = false;      // true while the table is being filled
};

#endif