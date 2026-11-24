#ifndef ORDERLISTWINDOW_H
#define ORDERLISTWINDOW_H

#include <QDialog>
#include <string>
#include "services/OrderService.h"

class QTableWidget;
class QPushButton;
class OrderRepository;
class ApprovalRepository;

// Cashier screen: paid orders of the last 7 days with their approval status.
// It only shows data. The rules are in OrderService.
class OrderListWindow : public QDialog
{
    Q_OBJECT

public:
    OrderListWindow(OrderRepository& orders, ApprovalRepository& approvals,
                    QWidget* parent = nullptr);

    // the order the cashier chose to edit
    std::string selectedOrderId() const { return chosenId; }

private slots:
    void refresh();
    void updateButtons();
    void requestSelected();
    void editSelected();

private:
    std::string currentId() const;                           // "" when no row is selected
    std::string statusOf(const std::string& orderId) const;  // pending, approved, rejected, used or none

    OrderRepository& orders;
    ApprovalRepository& approvals;
    OrderService service;              // only used for requestApproval
    QTableWidget* table;
    QPushButton* requestButton;
    QPushButton* editButton;
    std::string chosenId;
};

#endif