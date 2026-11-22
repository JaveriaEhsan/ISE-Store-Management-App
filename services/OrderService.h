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
    std::string customerName;
    std::string customerContact;            // empty if not given
    std::string paymentMode;                // PAY_CASH, PAY_CARD or PAY_WALLET
    std::string merchant;                   // for example ABL; empty for cash
    std::string status = STATUS_PAID;
};

struct CheckoutResult
{
    Order order;
    std::vector<OrderDetail> details;
    Bill bill;
    BillTotals totals;
};

class OrderService
{
private:
    std::vector<BillItem> cart;
    BillCalculator calculator;
    std::string resumedOrderId;     // the held order that is being completed, "" if none

public:
    bool addItem(const Product& product, double quantity, std::string& error);
    bool removeItem(const std::string& barcode);
    bool setQuantity(const Product& p, double quantity, std::string& error);
    void clear();

    const std::vector<BillItem>& getItems() const;
    BillTotals getTotals() const;


    bool checkout(const CheckoutInfo& info,
        IProductRepository& products,
        OrderRepository& orders,
        OrderDetailRepository& orderDetails,
        BillRepository& bills,
        CheckoutResult& result,
        std::string& error);

    static bool isValidStatus(const std::string& status);
    static bool isValidPaymentMode(const std::string& mode);
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