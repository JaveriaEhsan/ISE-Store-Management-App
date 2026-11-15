#ifndef ORDERREPOSITORY_H   
#define ORDERREPOSITORY_H

#include "../models/order.h"   // gives the Order struct

class OrderRepository
{
public:
    bool save(const Order& order);   // writes one order line to file
};

#endif