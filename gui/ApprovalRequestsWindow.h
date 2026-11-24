#ifndef APPROVALREQUESTSWINDOW_H
#define APPROVALREQUESTSWINDOW_H

#include <QDialog>
#include <string>

class QTableWidget;
class QLabel;
class ApprovalRepository;
class OrderRepository;

// Manager screen: lists pending edit requests and lets the manager approve or reject one.
// It only shows data and calls the repository. The rules live in OrderService.
class ApprovalRequestsWindow : public QDialog
{
    Q_OBJECT

public:
    ApprovalRequestsWindow(ApprovalRepository& approvals, OrderRepository& orders,
                           QWidget* parent = nullptr);

private slots:
    void refresh();
    void approveSelected();
    void rejectSelected();

private:
    void decide(const std::string& newStatus);
    std::string selectedRequestId() const;     // "" when no row is selected

    ApprovalRepository& approvals;
    OrderRepository& orders;
    QTableWidget* table;
    QLabel* emptyLabel;
};

#endif