#ifndef CASHIERWINDOW_H
#define CASHIERWINDOW_H

#include <QWidget>
#include <QEvent>
#include <QTableWidgetItem>
#include <QDoubleSpinBox>
#include <string>
#include "data/IProductRepository.h"
#include "data/OrderRepository.h"
#include "data/OrderDetailRepository.h"
#include "data/BillRepository.h"
#include "services/OrderService.h"

class QLineEdit;
class QTableWidget;
class QLabel;

class CashierWindow : public QWidget
{
    Q_OBJECT

public:
    CashierWindow(IProductRepository& products, OrderRepository& orders,
                  OrderDetailRepository& orderDetails, BillRepository& bills,
                  QWidget* parent = nullptr);

    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void onAddProduct();
    void onDescription();
    void onRemoveItem();
    void onGenerateBill();
    void onHoldOrder();
    void onResumeOrder();
    void onCancelOrder();
    void onBack();

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

    IProductRepository& products;
    OrderRepository& orders;
    OrderDetailRepository& orderDetails;
    BillRepository& bills;
    OrderService service;

    QLineEdit* nameEdit;
    QLineEdit* contactEdit;
    QTableWidget* table;
    QLabel* totalsLabel;
};

#endif