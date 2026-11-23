#ifndef ORDERDETAILREPOSITORY_H
#define ORDERDETAILREPOSITORY_H

#include "../models/orderDetail.h"   
#include <vector>
#include <string>

class OrderDetailRepository
{
public:
    bool save(const OrderDetail& detail);   // writes one line to file
    std::vector<OrderDetail> findByOrderId(const std::string& orderId) const;
    bool removeByOrderId(const std::string& orderId);

    // Swaps all lines of one order for the new ones (used when an order is edited)
    bool replaceForOrder(const std::string& orderId,
                         const std::vector<OrderDetail>& details);
};

#endif