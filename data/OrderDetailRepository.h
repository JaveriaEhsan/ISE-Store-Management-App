#ifndef ORDERDETAILREPOSITORY_H
#define ORDERDETAILREPOSITORY_H

#include "../models/orderDetail.h"   // gives the OrderDetail struct
#include <vector>
#include <string>

class OrderDetailRepository
{
public:
    bool save(const OrderDetail& detail);   // writes one line to file
    std::vector<OrderDetail> findByOrderId(const std::string& orderId) const;
    bool removeByOrderId(const std::string& orderId);
};

#endif