#ifndef ORDERREPOSITORY_H
#define ORDERREPOSITORY_H

#include "../models/order.h"
#include <vector>
#include <string>

// Reads and writes Order.txt (one line for each order)
class OrderRepository
{
public:
    // Adds a new order at the end of the file.
    // Returns false and puts the reason in error if the order cannot be saved.
    bool save(const Order& order, std::string& error);

    // Replaces the line of an existing order.
    // Returns false and puts the reason in error if the order is not valid or not found.
    bool update(const Order& order, std::string& error);

   // without an error text. Kept so older code still compiles.
    bool save(const Order& order)
    {
        std::string error;
        return save(order, error);
    }

    bool update(const Order& order)
    {
        std::string error;
        return update(order, error);
    }

    // Every order in the file
    std::vector<Order> getAll() const;

    // Finds one order by its id. Returns false if there is none.
    bool findById(const std::string& orderId, Order& result) const;

    // Every order with this status, for example "on-hold"
    std::vector<Order> findByStatus(const std::string& status) const;

    // Deletes the line of this order. Returns false if the file cannot be written.
    bool removeById(const std::string& orderId);
};

#endif