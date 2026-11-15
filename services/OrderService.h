#ifndef ORDERSERVICE_H
#define ORDERSERVICE_H

#include <string>
#include <vector>
#include "BillCalculator.h"
#include "../models/product.h"
#include "../models/order.h"
#include "../models/orderDetail.h"
#include "../models/bill.h"
#include "../data/IProductRepository.h"
#include "../data/OrderRepository.h"
#include "../data/OrderDetailRepository.h"
#include "../data/BillRepository.h"

// What the cashier enters at the end of the order
struct CheckoutInfo
{
    string customerName;
    string customerContact;            // empty if not given
    string paymentMode;                // PAY_CASH, PAY_CARD or PAY_WALLET
    string merchant;                   // for example ABL; empty for cash
    string status = STATUS_PAID;
};

// What checkout created, so the GUI can show the bill
struct CheckoutResult
{
    Order order;
    vector<OrderDetail> details;
    Bill bill;
    BillTotals totals;
};

class OrderService
{
private:
    vector<BillItem> cart;
    BillCalculator calculator;

public:
    bool addItem(const Product& product, double quantity, string& error);
    bool removeItem(const string& barcode);
    void clear();

    const vector<BillItem>& getItems() const;
    BillTotals getTotals() const;

    // Builds and saves the order, its details and the bill, then reduces stock
    bool checkout(const CheckoutInfo& info,
        IProductRepository& products,
        OrderRepository& orders,
        OrderDetailRepository& orderDetails,
        BillRepository& bills,
        CheckoutResult& result,
        string& error);

    static bool isValidStatus(const string& status);
    static bool isValidPaymentMode(const string& mode);
};

#endif