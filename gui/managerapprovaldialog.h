#ifndef MANAGERAPPROVALDIALOG_H
#define MANAGERAPPROVALDIALOG_H

#include <QDialog>
#include <string>

class QTableWidget;
class QLabel;
class ApprovalRepository;
class OrderRepository;
class OrderDetailRepository;

// Manager screen: pending requests to edit a paid order.
// It only shows data and calls the repositories. The 7-day rule is in OrderService.
class ManagerApprovalDialog : public QDialog
{
    Q_OBJECT

public:
    ManagerApprovalDialog(ApprovalRepository& approvals, OrderRepository& orders,
                          OrderDetailRepository& orderDetails, QWidget* parent = nullptr);

private slots:
    void refresh();          // reload the pending list
    void showItems();        // show the items of the selected order
    void approveClicked();
    void rejectClicked();

private:
    void decide(const std::string& newStatus);
    std::string selectedRequestId() const;     // "" when nothing is selected
    std::string selectedOrderId() const;

    ApprovalRepository& approvals;
    OrderRepository& orders;
    OrderDetailRepository& orderDetails;

    QTableWidget* table;
    QLabel* itemsLabel;
    QLabel* emptyLabel;
};

#endif