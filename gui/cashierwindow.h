#ifndef CASHIERWINDOW_H
#define CASHIERWINDOW_H

#include <QWidget>
#include <QEvent>
#include <QTableWidgetItem>
#include <QDoubleSpinBox>
#include <string>
#include "data/IProductRepository.h"
#include "data/ISeasonRepository.h"
#include "data/OrderRepository.h"
#include "data/OrderDetailRepository.h"
#include "data/BillRepository.h"
#include "data/BillDetailRepository.h"
#include "data/ApprovalRepository.h"
#include "services/OrderService.h"

class QLineEdit;
class QTableWidget;
class QLabel;
class QPushButton;

class CashierWindow : public QWidget
{
    Q_OBJECT

public:
    CashierWindow(IProductRepository& products, OrderRepository& orders,
                  OrderDetailRepository& orderDetails, BillRepository& bills,
                  BillDetailRepository& billDetails, ISeasonRepository& seasons,
                  ApprovalRepository& approvals, QWidget* parent = nullptr);

    bool eventFilter(QObject* watched, QEvent* event) override;

    // Opens a held or paid order in this window. onEditExisting calls it.
    bool startEdit(const std::string& orderId);

private slots:
    void onAddProduct();
    void onDescription();
    void onRemoveItem();
    void onGenerateBill();
    void onHoldOrder();
    void onResumeOrder();
    void onCancelOrder();
    void onBack();
    void onEditExisting();

private:
    void refreshTable();
    void showTotals();
    void resetScreen();
    void showNotFound(const std::string& barcode);
    bool isValidName(const QString& name) const;
    bool isValidContact(const QString& customerContact) const;
    bool readCustomer(std::string& name, std::string& contact);  // validates both
    void onQuantitySpin(const std::string& barcode, double value, QDoubleSpinBox* spin);
    bool askBarcode(std::string& barcode);
    void showProduct(const Product& p);
    void setEditMode(bool on);

    // the order of these members must match the constructor's initialiser list
    IProductRepository& products;
    OrderRepository& orders;
    OrderDetailRepository& orderDetails;
    BillRepository& bills;
    BillDetailRepository& billDetails;
    ISeasonRepository& seasons;
    ApprovalRepository& approvals;
    OrderService service;
    std::string editingOrderId;      // "" = normal order, else the paid order being edited

    QLineEdit* nameEdit = nullptr;
    QLineEdit* contactEdit = nullptr;
    QTableWidget* table = nullptr;
    QLabel* totalsLabel = nullptr;
    QLabel* subtitle = nullptr;
    QPushButton* billButton = nullptr;
    QPushButton* holdButton = nullptr;
    QPushButton* resumeButton = nullptr;
    QPushButton* cancelButton = nullptr;
    QPushButton* editExistingButton = nullptr;
};

#endif