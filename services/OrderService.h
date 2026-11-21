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

struct CheckoutInfo
{
    string customerName;
    string customerContact;            // empty if not given
    string paymentMode;                // PAY_CASH, PAY_CARD or PAY_WALLET
    string merchant;                   // for example ABL; empty for cash
    string status = STATUS_PAID;
};

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
    std::string resumedOrderId;     // the held order that is being completed, "" if none

public:
    bool addItem(const Product& product, double quantity, string& error);
    bool removeItem(const string& barcode);
    bool setQuantity(const Product& p, double quantity, string& error);
    void clear();

    const vector<BillItem>& getItems() const;
    BillTotals getTotals() const;


    bool checkout(const CheckoutInfo& info,
        IProductRepository& products,
        OrderRepository& orders,
        OrderDetailRepository& orderDetails,
        BillRepository& bills,
        CheckoutResult& result,
        string& error);

    static bool isValidStatus(const string& status);
    static bool isValidPaymentMode(const string& mode);
    // Saves the cart as an on-hold order. Stock does not change. Clears the cart.
    bool holdOrder(const std::string& name, const std::string& contact,
        OrderRepository& orders, OrderDetailRepository& details,
        std::string& orderId, std::string& error);

    // Loads the items of a held order into the cart.
    // Returns false if nothing could be loaded. "error" lists items that were skipped.
    bool resumeOrder(const std::string& orderId, IProductRepository& products,
        OrderDetailRepository& details, std::string& error);
};

#endif