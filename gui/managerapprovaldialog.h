#include <QDialog>
#include <string>
#include "data/ApprovalRepository.h"
#include "data/OrderRepository.h"
#include "data/OrderDetailRepository.h"
#include "data/IProductRepository.h"
#include "data/BillRepository.h"
#include "data/BillDetailRepository.h"

class QTableWidget;
class QLabel;
class QTextEdit;

class ManagerApprovalDialog : public QDialog
{
    Q_OBJECT

public:
    ManagerApprovalDialog(ApprovalRepository& approvals, OrderRepository& orders,
                          OrderDetailRepository& orderDetails, IProductRepository& products,
                          BillRepository& bills, BillDetailRepository& billDetails,
                          QWidget* parent = nullptr);

private slots:
    void refresh();
    void showItems();
    void approveClicked();
    void rejectClicked();

private:
    void decide(const std::string& newStatus);
    std::string selectedRequestId() const;
    std::string selectedOrderId() const;

    ApprovalRepository& approvals;
    OrderRepository& orders;
    OrderDetailRepository& orderDetails;
    IProductRepository& products;
    BillRepository& bills;
    BillDetailRepository& billDetails;

    QTableWidget* table;
    QLabel* emptyLabel;
    QTextEdit* itemsLabel;
};