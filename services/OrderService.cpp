#include "OrderService.h"
#include "IdGenerator.h"
#include <sstream>
#include <ctime>

using namespace std;

static string currentDate()
{
    time_t now = time(nullptr);
    tm* t = localtime(&now);
    char buffer[16];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", t);
    return buffer;
}

static string currentTime()
{
    time_t now = time(nullptr);
    tm* t = localtime(&now);
    char buffer[16];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", t);
    return buffer;
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

    // quantity already added to this order for the same product
    int index = -1;
    for (size_t i = 0; i < cart.size(); i++)
    {
        if (cart[i].barcode == p.getBarcode())
        {
            index = (int)i;
        }
    }
    double inCart = (index >= 0) ? cart[index].quantity : 0;

    if (!p.hasStock(inCart + quantity))
    {
        ostringstream msg;
        msg << "Not enough stock. Available: " << (p.getQuantity() - inCart);
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
    if (!p.hasStock(quantity))   // equal to the stock is allowed
    {
        ostringstream msg;
        msg << "Not enough stock. Available: " << p.getQuantity();
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
}

const vector<BillItem>& OrderService::getItems() const
{
    return cart;
}

BillTotals OrderService::getTotals() const
{
    return calculator.calculate(cart);
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

bool OrderService::checkout(const CheckoutInfo& info,
    IProductRepository& products,
    OrderRepository& orders,
    OrderDetailRepository& orderDetails,
    BillRepository& bills,
    CheckoutResult& result,
    string& error)
{
    error = "";


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
    r.totals = calculator.calculate(cart);

    IdGenerator ids;

    r.order.orderId = ids.nextOrderId();
    r.order.date = currentDate();
    r.order.time = currentTime();
    r.order.customerName = info.customerName;
    r.order.customerContact = info.customerContact;
    r.order.status = info.status;
    r.order.paymentAmount = r.totals.totalPayable;
    r.order.paymentMode = info.paymentMode;
    r.order.merchant = info.merchant;

    r.bill.billId = ids.nextBillId();
    r.bill.customerName = info.customerName;
    r.bill.orderId = r.order.orderId;
    r.bill.totalWithoutTax = r.totals.totalWithoutTax;
    r.bill.totalTax = r.totals.totalTax;
    r.bill.totalDiscount = r.totals.totalDiscount;
    r.bill.totalPayable = r.totals.totalPayable;
    r.bill.status = info.status;

    for (size_t i = 0; i < cart.size(); i++)
    {
        OrderDetail d;
        d.orderId = r.order.orderId;
        d.productId = cart[i].productId;
        d.barcode = cart[i].barcode;
        d.quantity = cart[i].quantity;
        r.details.push_back(d);

        BillLine line;
        line.acronym = cart[i].acronym;
        line.barcode = cart[i].barcode;
        line.unitPrice = cart[i].unitPrice;
        line.discountPerUnit = cart[i].discountPerUnit;
        line.taxPerUnit = cart[i].taxPerUnit;
        line.quantity = cart[i].quantity;
        r.bill.lines.push_back(line);
    }

    //  save 
    if (!orders.save(r.order))
    {
        error = "Could not save the order.";
        return false;
    }
    for (size_t i = 0; i < r.details.size(); i++)
    {
        if (!orderDetails.save(r.details[i]))
        {
            error = "Could not save the order details.";
            return false;
        }
    }
    string billError;
    if (!bills.save(r.bill, billError))
    {
        error = billError.empty() ? "Could not save the bill." : billError;
        return false;
    }

    if (!resumedOrderId.empty())          // this order came from a held order
    {
        orders.removeById(resumedOrderId);
        orderDetails.removeByOrderId(resumedOrderId);
    }

    // the files are saved, so this order is finished 
    vector<BillItem> sold = cart;
    result = r;
    clear();

    //  reduce stock (only when the order is paid)
    if (info.status == STATUS_PAID)
    {
        string failed = "";
        for (size_t i = 0; i < found.size(); i++)
        {
            found[i].reduceStock(sold[i].quantity);
            if (!products.update(found[i]))
            {
                failed += " " + sold[i].acronym;
            }
        }
        if (!failed.empty())
        {
            error = "Order saved, but the stock could not be updated for:" + failed;
            return false;
        }
    }

    return true;
}

bool OrderService::holdOrder(const string& name, const string& contact,
    OrderRepository& orders, OrderDetailRepository& details,
    string& orderId, string& error)
{
    if (cart.empty()) { error = "The order is empty."; return false; }

    time_t now = time(nullptr);
    tm* lt = localtime(&now);
    char date[20], clock[20];
    strftime(date, sizeof(date), "%Y-%m-%d", lt);
    strftime(clock, sizeof(clock), "%H:%M", lt);

    IdGenerator ids;
    orderId = ids.nextOrderId();

    Order o;
    o.orderId = orderId;
    o.date = currentDate();
    o.time = currentTime();
    
    o.customerName = name;
    o.customerContact = contact.empty() ? "-" : contact;
    o.status = STATUS_ON_HOLD;
    o.paymentAmount = getTotals().totalPayable;
    o.paymentMode = "-";
    o.merchant = "-";

    if (!orders.save(o)) { error = "Could not save the order."; return false; }

    for (const BillItem& it : cart)
    {
        OrderDetail d;
        d.orderId = orderId;
        d.productId = it.productId;
        d.barcode = it.barcode;
        d.quantity = it.quantity;
        if (!details.save(d)) { error = "Could not save the order items."; return false; }
    }

    // this order was resumed earlier: the new record replaces the old one
    if (!resumedOrderId.empty())
    {
        orders.removeById(resumedOrderId);
        details.removeByOrderId(resumedOrderId);
    }

    clear();
    return true;
}

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

    if (cart.empty()) return false;
    resumedOrderId = orderId;                      // set AFTER clear()
    return true;
}