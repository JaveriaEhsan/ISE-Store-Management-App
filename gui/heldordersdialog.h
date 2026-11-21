#ifndef HELDORDERSDIALOG_H
#define HELDORDERSDIALOG_H

#include <QDialog>
#include <vector>
#include "models/order.h"

class QTableWidget;

// Shows the held orders. After exec() == Accepted, selectedOrder() is the chosen one.
class HeldOrdersDialog : public QDialog
{
    Q_OBJECT

public:
    HeldOrdersDialog(const std::vector<Order>& orders, QWidget* parent = nullptr);
    Order selectedOrder() const;

private slots:
    void onResume();

private:
    std::vector<Order> held;
    QTableWidget* table;
    int chosenRow = -1;
};

#endif