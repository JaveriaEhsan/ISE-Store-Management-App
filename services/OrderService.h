#ifndef ORDERSERVICE_H
#define ORDERSERVICE_H

#include <map>
#include <string>
#include <vector>
#include "BillCalculator.h"
#include "../models/product.h"
#include "../models/order.h"
#include "../models/orderDetail.h"
#include "../models/bill.h"
#include "../data/IProductRepository.h"
#include "../data/ISeasonRepository.h"
#include "../data/OrderRepository.h"
#include "../data/OrderDetailRepository.h"
#include "../data/BillRepository.h"
#include "../data/BillDetailRepository.h"

struct CheckoutInfo
{
    std::string customerName;
    std::string customerContact;        // empty if not given
    std::string paymentMode;            // PAY_CASH, PAY_CARD or PAY_WALLET
    std::string merchant;               // for example ABL; empty for cash
    std::string status = STATUS_PAID;
    double specialPercent = 0;          // 0 to 8
    double seasonalPercent = 0;         // from currentSeasonalPercent()
    double amountReceived = 0;          // must be at least the payable amount
};

struct CheckoutResult
{
    Order order;
    std::vector<OrderDetail> details;
    Bill bill;
    BillTotals totals;
    bool editedPaidOrder = false;    // true when a paid order was edited
    std::string replacedBillId;         // the bill this edit replaces ("" if none)
};

class OrderService
{
private:
    std::vector<BillItem> cart;
    BillCalculator calculator;
    std::string resumedOrderId;                   // held order being completed, "" if none

    // editing a PAID order
    std::string editedOrderId;                    // "" when no paid order is being edited
    Order editedOrder;
    std::map<std::string, double> originalQty;    // barcode -> quantity already taken from stock

    double availableFor(const Product& p) const;
    BillTotals totalsFor(const CheckoutInfo& info) const;
    bool validate(const CheckoutInfo& info, std::string& error) const;
    Order makeOrder(const std::string& orderId, const std::string& date, const std::string& time,const CheckoutInfo& info, const BillTotals& t) const;std::vector<OrderDetail> makeDetails(const std::string& orderId) const;
    Bill makeBill(const std::string& billId, const std::string& orderId,const std::string& date, const std::string& time,const CheckoutInfo& info, const BillTotals& t) const;
    bool saveBill(const Bill& bill, BillRepository& bills,BillDetailRepository& billDetails, std::string& error) const;



public:
    bool addItem(const Product& product, double quantity, std::string& error);
    bool removeItem(const std::string& barcode);
    bool setQuantity(const Product& p, double quantity, std::string& error);
    void clear();

    const std::vector<BillItem>& getItems() const;
    BillTotals getTotals() const;

    // Highest percentage of the seasons that are active today (0 if none).
    // When ISeasonRepository gets findActiveSeason, this body becomes one call.
    double currentSeasonalPercent(const ISeasonRepository& seasons) const;

    // Saves order, details, bill and bill details, then reduces stock.
    // If a paid order was opened with openForEdit, this saves the edit instead.
    bool checkout(const CheckoutInfo& info,IProductRepository& products,OrderRepository& orders,OrderDetailRepository& orderDetails,BillRepository& bills,BillDetailRepository& billDetails,CheckoutResult& result,std::string& error);

    static bool isValidStatus(const std::string& status);
    static bool isValidPaymentMode(const std::string& mode);

    // Saves the cart as an on-hold order. Stock does not change. Clears the cart.
    bool holdOrder(const std::string& name, const std::string& contact,OrderRepository& orders, OrderDetailRepository& details,std::string& orderId, std::string& error);

    // Loads the items of a held order into the cart.
    bool resumeOrder(const std::string& orderId, IProductRepository& products,OrderDetailRepository& details, std::string& error);

    // ----- editing an existing order -----
    // false for archived orders, and for paid orders older than 7 days
    static bool canEdit(const Order& order, std::string& reason);
   
    // only paid orders need a manager's approval
    static bool needsApproval(const Order& order) { return order.status == STATUS_PAID; }
   
    // paid and within PAID_ORDER_EDIT_DAYS days
    static bool canRequestEdit(const Order& order, std::string& reason);

    // on-hold / in-progress: same as resumeOrder. paid: needs approved == true.
    bool loadOrderForEdit(const std::string& orderId, bool approved,IProductRepository& products,OrderRepository& orders, OrderDetailRepository& details,std::string& error);
    bool isEditingPaidOrder() const { return !editedOrderId.empty(); }
    // Saves the edit of a paid order. checkout calls this too when a paid order is open.
    bool saveEditedOrder(const CheckoutInfo& info, IProductRepository& products,OrderRepository& orders, OrderDetailRepository& orderDetails,BillRepository& bills, BillDetailRepository& billDetails,
        CheckoutResult& result, std::string& error);

    // How much the stock changes per barcode when an order is edited:
    // positive = take more from the stock, negative = give back.
    static std::map<std::string, double> stockDifference(const std::map<std::string, double>& before,const std::vector<BillItem>& after);
    
};

#endif