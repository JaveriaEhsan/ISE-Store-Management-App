#ifndef ORDERSERVICE_H
#define ORDERSERVICE_H

#include <string>
#include <vector>
#include "BillCalculator.h"
#include "../models/product.h"
#include "../models/order.h"   // check the real file name in the models folder

class OrderService
{
private:
    vector<BillItem> cart;
    BillCalculator calculator;

public:
    // Adds the product if stock allows it. On failure, error says why.
    bool addItem(const Product& product, double quantity, string& error);
    bool removeItem(const string& barcode);
    void clear();

    const vector<BillItem>& getItems() const;
    BillTotals getTotals() const;

    static bool isValidStatus(const string& status);
};

#endif