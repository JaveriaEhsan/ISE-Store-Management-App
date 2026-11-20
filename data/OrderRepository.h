#ifndef ORDERREPOSITORY_H   
#define ORDERREPOSITORY_H

#include "../models/order.h"   // gives the Order struct
#include<vector>
#include<string>
class OrderRepository
{
public:
    bool save(const Order& order);   // writes one order line to file
    std::vector<Order> findByStatus(const std::string& status) const;
    bool removeById(const std::string& orderId);
};

#endif