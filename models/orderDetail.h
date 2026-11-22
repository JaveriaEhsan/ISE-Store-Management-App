#ifndef ORDERDETAIL_H
#define ORDERDETAIL_H

#include <string>

// One row of OrderDetail.txt (one product inside an order)
struct OrderDetail
{
    std::string orderId;          // the order id from Order.txt
    std::string productId;
    std::string barcode;
    double quantity = 0;
};

#endif