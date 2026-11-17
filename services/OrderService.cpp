#include "OrderService.h"
#include "IdGenerator.h"
#include <sstream>
#include <ctime>

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

void OrderService::clear()
{
    cart.clear();
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
        line.unitPrice = cart[i].unitPrice;
        line.discountPerUnit = cart[i].discountPerUnit;
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
    if (!bills.save(r.bill))
    {
        error = "Could not save the bill.";
        return false;
    }

    // the files are saved, so this order is finished 
    vector<BillItem> sold = cart;
    result = r;
    cart.clear();

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