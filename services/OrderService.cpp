#include "OrderService.h"
#include <sstream>

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