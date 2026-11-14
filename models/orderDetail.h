#ifndef ORDERDETAIL_H
#define ORDERDETAIL_H

#include <string>
using namespace std;

// One row of OrderDetail.txt (one product inside an order)
struct OrderDetail
{
    string orderId;          // the order id from Order.txt
    string productId;
    string barcode;
    double quantity = 0;
};

#endif