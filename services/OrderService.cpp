#include "OrderService.h"
#include "IdGenerator.h"
#include "DateUtil.h"
#include "../config.h"
#include <sstream>
#include <cmath>
#include <map>
#include <vector>

using namespace std;

// ---------------------------------------------------------------- cart

double OrderService::availableFor(const Product& p) const
{
    // when a paid order is edited, what it already took from the stock counts as available
    double extra = 0;
    map<string, double>::const_iterator it = originalQty.find(p.getBarcode());
    if (it != originalQty.end()) extra = it->second;
    return p.getQuantity() + extra;
}

bool OrderService::addItem(const Product& p, double quantity, string& error)
{
    error = "";

    if (quantity <= 0)
    {
        error = "Quantity must be greater than 0.";
        return false;
    }
    if (p.getUnit() == "count" && quantity != (long long)quantity)
    {
        error = "This product is sold by count, so use a whole number.";
        return false;
    }

    int index = -1;
    for (size_t i = 0; i < cart.size(); i++)
    {
        if (cart[i].barcode == p.getBarcode())
        {
            index = (int)i;
        }
    }
    double inCart = (index >= 0) ? cart[index].quantity : 0;
    double available = availableFor(p);

    if (inCart + quantity > available)   // equal to the stock is allowed
    {
        ostringstream msg;
        msg << "Not enough stock. Available: " << (available - inCart);
        error = msg.str();
        return false;
    }

    if (index >= 0)
    {
        cart[index].quantity += quantity;
        return true;
    }

    BillItem item;
    item.productId = p.getId();
    item.barcode = p.getBarcode();
    item.acronym = p.getAcronym();
    item.unitPrice = p.getPrice();
    item.taxPerUnit = p.getTax();
    item.discountPerUnit = p.getDiscount();
    item.quantity = quantity;
    cart.push_back(item);
    return true;
}

bool OrderService::removeItem(const string& barcode)
{
    for (size_t i = 0; i < cart.size(); i++)
    {
        if (cart[i].barcode == barcode)
        {
            cart.erase(cart.begin() + i);
            return true;
        }
    }
    return false;
}

bool OrderService::setQuantity(const Product& p, double quantity, string& error)
{
    error = "";

    if (quantity <= 0)
    {
        error = "Quantity must be greater than 0.";
        return false;
    }
    if (p.getUnit() == "count" && quantity != (long long)quantity)
    {
        error = "This product is sold by count, so use a whole number.";
        return false;
    }
    if (quantity > availableFor(p))
    {
        ostringstream msg;
        msg << "Not enough stock. Available: " << availableFor(p);
        error = msg.str();
        return false;
    }

    for (size_t i = 0; i < cart.size(); i++)
    {
        if (cart[i].barcode == p.getBarcode())
        {
            cart[i].quantity = quantity;
            return true;
        }
    }

    error = "This product is not in the order.";
    return false;
}

void OrderService::clear()
{
    cart.clear();
    resumedOrderId.clear();
    editedOrderId.clear();
    editedOrder = Order();
    originalQty.clear();
}

const vector<BillItem>& OrderService::getItems() const
{
    return cart;
}

BillTotals OrderService::getTotals() const
{
    return calculator.calculate(cart);
}

BillTotals OrderService::getTotals(double seasonalPercent, double specialPercent) const
{
    CheckoutInfo info;
    info.seasonalPercent = seasonalPercent;
    info.specialPercent = specialPercent;
    return totalsFor(info);
}

bool OrderService::isValidStatus(const string& status)
{
    return status == STATUS_IN_PROGRESS || status == STATUS_ON_HOLD
        || status == STATUS_PAID || status == STATUS_ARCHIVED;
}

bool OrderService::isValidPaymentMode(const string& mode)
{
    return mode == PAY_CASH || mode == PAY_CARD || mode == PAY_WALLET;
}

// ---------------------------------------------------------------- seasons and totals

double OrderService::currentSeasonalPercent(const ISeasonRepository& seasons) const
{
    string today = dateKey(todayDate());
    double best = 0;

    vector<Season> all = seasons.getAll();
    for (size_t i = 0; i < all.size(); i++)
    {
        if (!all[i].isActive) continue;

        string start = dateKey(all[i].startDate);
        string end = dateKey(all[i].endDate);
        if (start.empty() || end.empty()) continue;

        if (start <= today && today <= end && all[i].discountPercent > best)
        {
            best = all[i].discountPercent;
        }
    }
    return best;
}

BillTotals OrderService::totalsFor(const CheckoutInfo& info) const
{
    SeasonalDiscount seasonal(info.seasonalPercent);
    SpecialDiscount special(info.specialPercent);

    vector<const IDiscountRule*> rules;
    rules.push_back(&seasonal);
    rules.push_back(&special);

    return calculator.calculate(cart, rules);
}

bool OrderService::validate(const CheckoutInfo& info, string& error) const
{
    if (cart.empty())
    {
        error = "The order is empty.";
        return false;
    }
    if (info.customerName.empty())
    {
        error = "Customer name is required.";
        return false;
    }
    if (!isValidStatus(info.status))
    {
        error = "Invalid order status.";
        return false;
    }
    if (!isValidPaymentMode(info.paymentMode))
    {
        error = "Payment mode must be cash, card or wallet.";
        return false;
    }
    if (info.paymentMode != PAY_CASH && info.merchant.empty())
    {
        error = "Merchant name is required for card and wallet payments.";
        return false;
    }
    if (!SpecialDiscount::isValidPercent(info.specialPercent))
    {
        error = "Special discount must be between 0 and 8 percent.";
        return false;
    }

    BillTotals totals = totalsFor(info);
    if (totals.totalPayable < 0)
    {
        error = "The payable amount cannot be negative. Check the product discounts.";
        return false;
    }
    if (!calculator.isEnough(info.amountReceived, totals.totalPayable))
    {
        error = "Amount received must be at least the payable amount.";
        return false;
    }
    return true;
}

// ---------------------------------------------------------------- building records

Order OrderService::makeOrder(const string& orderId, const string& date, const string& time,
    const CheckoutInfo& info, const BillTotals& t) const
{
    Order o;
    o.orderId = orderId;
    o.date = date;
    o.time = time;
    o.customerName = info.customerName;
    o.customerContact = info.customerContact.empty() ? "-" : info.customerContact;
    o.status = info.status;
    o.paymentAmount = t.totalPayable;
    o.paymentMode = info.paymentMode;
    o.merchant = info.merchant.empty() ? "-" : info.merchant;
    return o;
}

vector<OrderDetail> OrderService::makeDetails(const string& orderId) const
{
    vector<OrderDetail> list;
    for (size_t i = 0; i < cart.size(); i++)
    {
        OrderDetail d;
        d.orderId = orderId;
        d.productId = cart[i].productId;
        d.barcode = cart[i].barcode;
        d.quantity = cart[i].quantity;
        list.push_back(d);
    }
    return list;
}

Bill OrderService::makeBill(const string& billId, const string& orderId,
    const string& date, const string& time,
    const CheckoutInfo& info, const BillTotals& t) const
{
    Bill b;
    b.billId = billId;
    b.customerName = info.customerName;
    b.orderId = orderId;
    b.totalWithoutTax = t.totalWithoutTax;
    b.totalTax = t.totalTax;
    b.totalDiscount = t.totalDiscount;          // product discounts
    b.totalPayable = t.totalPayable;
    b.status = info.status;
    b.amountReceived = info.amountReceived;
    b.balanceReturned = calculator.balance(info.amountReceived, t.totalPayable);
    b.seasonalDiscount = t.seasonalAmount;      // PKR
    b.specialDiscount = t.specialAmount;        // PKR
    b.date = date;
    b.time = time;

    for (size_t i = 0; i < cart.size(); i++)
    {
        BillLine line;
        line.acronym = cart[i].acronym;
        line.barcode = cart[i].barcode;
        line.unitPrice = cart[i].unitPrice;
        line.discountPerUnit = cart[i].discountPerUnit;
        line.taxPerUnit = cart[i].taxPerUnit;
        line.quantity = cart[i].quantity;
        b.lines.push_back(line);
    }
    return b;
}

// Bill.txt gets one line, BillDetail.txt gets one line per product.
// If a detail line fails, the bill is cancelled so it is not left half saved.
bool OrderService::saveBill(const Bill& bill, BillRepository& bills,
    BillDetailRepository& billDetails, string& error) const
{
    if (!bills.save(bill, error))
    {
        if (error.empty()) error = "Could not save the bill.";
        return false;
    }
    for (size_t i = 0; i < bill.lines.size(); i++)
    {
        BillDetail d;
        d.billId = bill.billId;
        d.acronym = bill.lines[i].acronym;
        d.unitPrice = bill.lines[i].unitPrice;
        d.quantity = bill.lines[i].quantity;
        d.discount = bill.lines[i].discount();      // discount per unit x quantity

        if (!billDetails.save(d))
        {
            bills.updateStatus(bill.billId, STATUS_CANCELLED);   // never issued to the customer
            error = "Could not save the bill details. The bill was cancelled.";
            return false;
        }
    }
    return true;
}

// ---------------------------------------------------------------- checkout

bool OrderService::checkout(const CheckoutInfo& info,
    IProductRepository& products,
    OrderRepository& orders,
    OrderDetailRepository& orderDetails,
    BillRepository& bills,
    BillDetailRepository& billDetails,
    CheckoutResult& result,
    string& error,
    ApprovalRepository* approvals)
{
    error = "";

    if (!editedOrderId.empty())
    {
        return saveEditedOrder(info, products, orders, orderDetails,
            bills, billDetails, result, error, approvals);
    }
  

    if (!validate(info, error))
    {
        return false;
    }

    // check every product again, because stock may have changed since it was added
    vector<Product> found;
    for (size_t i = 0; i < cart.size(); i++)
    {
        Product p;
        if (!products.findByBarcode(cart[i].barcode, p))
        {
            error = "Product not found: " + cart[i].acronym;
            return false;
        }
        if (!p.hasStock(cart[i].quantity))
        {
            error = "Not enough stock for " + cart[i].acronym;
            return false;
        }
        found.push_back(p);
    }

    // build the records
    CheckoutResult r;
    r.totals = totalsFor(info);

    IdGenerator ids;
    r.order = makeOrder(ids.nextOrderId(), todayDate(), nowTime(), info, r.totals);
    r.details = makeDetails(r.order.orderId);
    r.bill = makeBill(ids.nextBillId(), r.order.orderId, r.order.date, r.order.time, info, r.totals);

    // save the new records first
    if (!orders.save(r.order, error))
    {
        return false;
    }
    for (size_t i = 0; i < r.details.size(); i++)
    {
        if (!orderDetails.save(r.details[i]))
        {
            orders.removeById(r.order.orderId);              // do not leave a half saved order
            orderDetails.removeByOrderId(r.order.orderId);
            error = "Could not save the order details.";
            return false;
        }
    }
    if (!saveBill(r.bill, bills, billDetails, error))
    {
        orders.removeById(r.order.orderId);                  // do not leave an order without a bill
        orderDetails.removeByOrderId(r.order.orderId);
        return false;
    }

    // the old held order is removed after the new records are saved
    string warning = "";
    if (!resumedOrderId.empty())
    {
        bool removedOrder = orders.removeById(resumedOrderId);
        bool removedItems = orderDetails.removeByOrderId(resumedOrderId);
        if (!removedOrder || !removedItems)
        {
            warning = "Order saved, but the old held order could not be removed completely.";
        }
    }

    // the files are saved, so this order is finished
    vector<BillItem> sold = cart;
    result = r;
    clear();

    // reduce stock (only when the order is paid)
    string failed = "";
    if (info.status == STATUS_PAID)
    {
        for (size_t i = 0; i < found.size(); i++)
        {
            found[i].reduceStock(sold[i].quantity);

            string updateError;
            if (!products.update(found[i], updateError))
            {
                failed += " " + sold[i].acronym;
                if (!updateError.empty()) failed += " (" + updateError + ")";
            }
        }
    }

    string message = warning;
    if (!failed.empty())
    {
        if (!message.empty()) message += " ";
        message += "Order saved, but the stock could not be updated for:" + failed;
    }
    if (!message.empty())
    {
        error = message;
        return false;      // result is filled, so the window can still show the bill
    }
    return true;
}

// ---------------------------------------------------------------- hold and resume

bool OrderService::holdOrder(const string& name, const string& contact,
    OrderRepository& orders, OrderDetailRepository& details,
    string& orderId, string& error)
{
    error.clear();
    if (cart.empty()) { error = "The order is empty."; return false; }
    if (!editedOrderId.empty()) { error = "A paid order cannot be put on hold."; return false; }

    IdGenerator ids;
    orderId = ids.nextOrderId();

    Order o;
    o.orderId = orderId;
    o.date = todayDate();
    o.time = nowTime();
    o.customerName = name;
    o.customerContact = contact.empty() ? "-" : contact;
    o.status = STATUS_ON_HOLD;
    o.paymentAmount = getTotals().totalPayable;     // discounts are added when the bill is made
    o.paymentMode = "-";
    o.merchant = "-";

    if (!orders.save(o, error)) { return false; }

    for (size_t i = 0; i < cart.size(); i++)
    {
        OrderDetail d;
        d.orderId = orderId;
        d.productId = cart[i].productId;
        d.barcode = cart[i].barcode;
        d.quantity = cart[i].quantity;
        if (!details.save(d))
        {
            orders.removeById(orderId);                  // do not leave a half saved hold
            details.removeByOrderId(orderId);
            error = "Could not save the order items.";
            return false;
        }
    }

    // this order was resumed earlier: the new record replaces the old one
    bool oldRemoved = true;
    if (!resumedOrderId.empty())
    {
        bool removedOrder = orders.removeById(resumedOrderId);
        bool removedItems = details.removeByOrderId(resumedOrderId);
        oldRemoved = removedOrder && removedItems;
    }

    clear();

    if (!oldRemoved)
    {
        error = "The order is on hold, but the old copy could not be removed.";
        return false;      // orderId is filled: the new hold is saved
    }
    return true;
}

// false only if nothing could be loaded.
// true with error NOT empty = WARNING: some lines could not be loaded and will be lost
// if the cashier saves. The window must show the text and ask for confirmation.
// If the cashier says no, call service.clear().
bool OrderService::resumeOrder(const string& orderId, IProductRepository& products,
    OrderDetailRepository& details, string& error)
{
    clear();
    error.clear();

    for (const OrderDetail& d : details.findByOrderId(orderId))
    {
        Product p;
        if (!products.findByBarcode(d.barcode, p))
        {
            error += d.barcode + ": product no longer exists\n";
            continue;
        }
        string why;
        if (!addItem(p, d.quantity, why))
            error += p.getAcronym() + ": " + why + "\n";
    }

    if (cart.empty())
    {
        if (error.empty()) error = "The order has no items.";
        clear();
        return false;
    }
    resumedOrderId = orderId;                      // set AFTER clear()
    return true;
}

// ---------------------------------------------------------------- editing an existing order

bool OrderService::canEdit(const Order& order, string& reason)
{
    reason.clear();

    if (order.status == STATUS_ARCHIVED)
    {
        reason = "Archived orders cannot be edited.";
        return false;
    }
    if (order.status == STATUS_PAID)
    {
        return canRequestEdit(order, reason);       // paid: only within 7 days
    }
    return true;   // on-hold and in-progress orders can always be edited
}

bool OrderService::loadOrderForEdit(const string& orderId,
    IProductRepository& products,
    OrderRepository& orders, OrderDetailRepository& details,
    const ApprovalRepository& approvals,
    string& error)
{
    clear();
    error.clear();

    Order order;
    if (!orders.findById(orderId, order))
    {
        error = "Order not found.";
        return false;
    }
    if (!canEdit(order, error))
    {
        return false;
    }

    if (order.status != STATUS_PAID)
    {
        return resumeOrder(orderId, products, details, error);   // held or in-progress (warning allowed)
    }

    if (!isApproved(orderId, approvals))
    {
        error = "Manager approval is required to edit a paid order.";
        return false;
    }

    // paid order: remember what it already took from the stock
    vector<OrderDetail> old = details.findByOrderId(orderId);
    for (size_t i = 0; i < old.size(); i++)
    {
        originalQty[old[i].barcode] += old[i].quantity;
    }
    editedOrderId = orderId;
    editedOrder = order;

    for (size_t i = 0; i < old.size(); i++)
    {
        Product p;
        if (!products.findByBarcode(old[i].barcode, p))
        {
            error += old[i].barcode + ": product no longer exists\n";
            continue;
        }
        string why;
        if (!addItem(p, old[i].quantity, why))
            error += p.getAcronym() + ": " + why + "\n";
    }

    // paid orders are strict: if any line failed to load, refuse,
    // because saving would silently drop those lines
    if (!error.empty() || cart.empty())
    {
        if (error.empty()) error = "The order has no items.";
        clear();
        return false;
    }
    return true;
}

bool OrderService::editedBillPayable(const BillRepository& bills, double& payable) const
{
    if (editedOrderId.empty()) return false;

    Bill b;
    if (!bills.findCurrentByOrderId(editedOrderId, b)) return false;

    payable = b.totalPayable;
    return true;
}

map<string, double> OrderService::stockDifference(const map<string, double>& before,
    const vector<BillItem>& after)
{
    map<string, double> diff;

    for (map<string, double>::const_iterator it = before.begin(); it != before.end(); ++it)
    {
        diff[it->first] -= it->second;
    }
    for (size_t i = 0; i < after.size(); i++)
    {
        diff[after[i].barcode] += after[i].quantity;
    }

    for (map<string, double>::iterator it = diff.begin(); it != diff.end(); )
    {
        if (fabs(it->second) < 1e-9) it = diff.erase(it);   // no change for this product
        else ++it;
    }
    return diff;
}

// For an edit, info.amountReceived is only the EXTRA money the cashier takes now
// (0 when the new total is lower). The new bill stores full figures:
//   amountReceived  = old payable + extra
//   balanceReturned = amountReceived - new payable
bool OrderService::saveEditedOrder(const CheckoutInfo& info, IProductRepository& products,
    OrderRepository& orders, OrderDetailRepository& orderDetails,
    BillRepository& bills, BillDetailRepository& billDetails,
    CheckoutResult& result, string& error,
    ApprovalRepository* approvals) {
    error.clear();

    if (editedOrderId.empty())
    {
        error = "No paid order is open for editing.";
        return false;
    }
    if (info.status != STATUS_PAID)
    {
        error = "A paid order stays paid.";
        return false;
    }
    // the approval must still be valid at the moment of saving
    Approval approval;
    if (approvals == nullptr
        || !approvals->findActiveByOrderId(editedOrderId, approval)
        || approval.status != APPROVAL_APPROVED)
    {
        error = "The manager's approval is missing or already used. Request a new approval.";
        return false;
    }

    // 0. the old bill is needed to work out the difference
    Bill oldBill;
    if (!bills.findCurrentByOrderId(editedOrderId, oldBill))
    {
        error = "The old bill of this order was not found. The edit cannot be saved.";
        return false;
    }

    BillTotals newTotals = totalsFor(info);
    double difference = calculator.difference(newTotals.totalPayable, oldBill.totalPayable);
    double extra = info.amountReceived;

    if (difference > 0 && !calculator.isEnough(extra, difference))
    {
        error = "The customer must pay at least the difference.";
        return false;
    }
    if (difference <= 0)
    {
        extra = 0;          // nothing to receive
    }

    // validate() checks "received >= payable", which does not apply to an edit
    CheckoutInfo checkInfo = info;
    checkInfo.amountReceived = newTotals.totalPayable;
    if (!validate(checkInfo, error))
    {
        return false;
    }

    // the new bill shows full figures
    CheckoutInfo billInfo = info;
    billInfo.amountReceived = calculator.paidAfterEdit(oldBill.totalPayable, extra);

    // 1. stock check, before anything is saved
    map<string, double> diff = stockDifference(originalQty, cart);
    vector<Product> changed;
    vector<double> amounts;
    for (map<string, double>::const_iterator it = diff.begin(); it != diff.end(); ++it)
    {
        Product p;
        if (!products.findByBarcode(it->first, p))
        {
            if (it->second > 0)
            {
                error = "Product not found: " + it->first;
                return false;
            }
            continue;   // a removed product that no longer exists: nothing to give back
        }
        if (it->second > 0 && !p.hasStock(it->second))
        {
            error = "Not enough stock for " + p.getAcronym();
            return false;
        }
        changed.push_back(p);
        amounts.push_back(it->second);
    }

    // 2. build the new records
    CheckoutResult r;
    r.totals = newTotals;
    r.editedPaidOrder = true;
    r.editDifference = difference;        // positive = collected, negative = refunded
    r.replacedBillId = oldBill.billId;

    // the order keeps its original date and time, so an edit does not extend the 7 days
    r.order = makeOrder(editedOrderId, editedOrder.date, editedOrder.time, info, r.totals);
    r.details = makeDetails(editedOrderId);

    // keep the old lines so they can be put back if the replace fails
    vector<OrderDetail> oldDetails = orderDetails.findByOrderId(editedOrderId);

    IdGenerator ids;
    r.bill = makeBill(ids.nextBillId(), editedOrderId, todayDate(), nowTime(), billInfo, r.totals);

    // 3. new records first
    if (!saveBill(r.bill, bills, billDetails, error))
    {
        return false;
    }

    string orderError;
    if (!orders.update(r.order, orderError))
    {
        bills.updateStatus(r.bill.billId, STATUS_CANCELLED);   // the new bill was never issued
        error = "Could not update the order. The edit was cancelled. " + orderError;
        return false;
    }
    if (!orderDetails.replaceForOrder(editedOrderId, r.details))
    {
        string restoreError;
        orders.update(editedOrder, restoreError);                          // old order back
        bool restored = orderDetails.replaceForOrder(editedOrderId, oldDetails);
        bills.updateStatus(r.bill.billId, STATUS_CANCELLED);               // new bill never issued
        error = restored
            ? "Could not update the order items. The edit was cancelled."
            : "Could not update the order items, and the old items could not be restored. Check OrderDetail.txt.";
        return false;
    }

    // 4. old records after the new ones are saved
    string warning = "";
    if (!bills.updateStatus(oldBill.billId, STATUS_EDITED))   // a real bill that was replaced
    {
        warning = "Order saved, but the old bill could not be marked as edited.";
    }
    string approvalError;
    if (!approvals->setStatus(approval.requestId, APPROVAL_USED, approvalError))
    {
        if (!warning.empty()) warning += " ";
        warning += "Order saved, but the approval could not be marked as used. " + approvalError;
    }
    // 5. stock: only the difference changes
    result = r;
    clear();

    string failed = "";
    for (size_t i = 0; i < changed.size(); i++)
    {
        if (amounts[i] > 0) changed[i].reduceStock(amounts[i]);
        else                changed[i].addStock(-amounts[i]);

        string updateError;
        if (!products.update(changed[i], updateError))
        {
            failed += " " + changed[i].getAcronym();
            if (!updateError.empty()) failed += " (" + updateError + ")";
        }
    }

    string message = warning;
    if (!failed.empty())
    {
        if (!message.empty()) message += " ";
        message += "Order saved, but the stock could not be updated for:" + failed;
    }
    if (!message.empty())
    {
        error = message;
        return false;      // result is filled, so the window can still show the bill
    }
    return true;
}

// ---------------------------------------------------------------- 7-day rule

bool OrderService::canRequestEdit(const Order& order, string& reason)
{
    reason.clear();

    if (order.status != STATUS_PAID)
    {
        reason = "Only paid orders need a manager's approval.";
        return false;
    }

    int days = 0;
    if (!daysBetween(order.date, todayDate(), days))
    {
        reason = "The date of this order is not valid.";
        return false;
    }
    if (days < 0)
    {
        reason = "The date of this order is in the future.";
        return false;
    }
    if (days > PAID_ORDER_EDIT_DAYS)
    {
        reason = "A paid order can only be edited within "
            + to_string(PAID_ORDER_EDIT_DAYS) + " days of payment.";
        return false;
    }
    return true;
}
// ---------------------------------------------------------------- approvals

bool OrderService::requestApproval(const string& orderId,
    OrderRepository& orders, ApprovalRepository& approvals,
    string& requestId, string& error)
{
    error.clear();
    requestId.clear();

    Order order;
    if (!orders.findById(orderId, order))
    {
        error = "Order not found.";
        return false;
    }
    if (!canRequestEdit(order, error))          // paid, and within the 7 days
    {
        return false;
    }

    Approval existing;
    if (approvals.findActiveByOrderId(orderId, existing))
    {
        error = (existing.status == APPROVAL_PENDING)
            ? "A request for this order is already waiting for the manager."
            : "This order is already approved for editing.";
        return false;
    }

    Approval a;
    a.requestId = approvals.nextId();
    a.orderId = orderId;
    a.requestDate = todayDate();
    a.requestTime = nowTime();
    a.status = APPROVAL_PENDING;
    a.decisionDate = "-";
    a.decisionTime = "-";

    if (!approvals.add(a, error))
    {
        return false;
    }
    requestId = a.requestId;
    return true;
}

bool OrderService::isApproved(const string& orderId, const ApprovalRepository& approvals)
{
    Approval a;
    return approvals.findActiveByOrderId(orderId, a) && a.status == APPROVAL_APPROVED;
}