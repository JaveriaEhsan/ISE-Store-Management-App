#ifndef ORDERDETAILREPOSITORY_H   
#define ORDERDETAILREPOSITORY_H

#include "../models/orderDetail.h"   // gives the OrderDetail struct

class OrderDetailRepository
{
public:
    bool save(const OrderDetail& detail);   // writes one line to file
};

#endif
