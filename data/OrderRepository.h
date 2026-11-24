#ifndef ORDERREPOSITORY_H
#define ORDERREPOSITORY_H

#include "../models/order.h"
#include <vector>
#include <string>

class OrderRepository
{
public:
    // Adds one line at the end. Returns false and fills error if the order is not valid.
    bool save(const Order& order, std::string& error);

    std::vector<Order> getAll() const;
    bool findById(const std::string& orderId, Order& result) const;
    std::vector<Order> findByStatus(const std::string& status) const;

    // Rewrites the line of this order id. Returns false and fills error if not valid or not found.
    bool update(const Order& order, std::string& error);

    bool removeById(const std::string& orderId);
};

#endif