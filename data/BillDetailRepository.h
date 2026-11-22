#ifndef BILLDETAILREPOSITORY_H
#define BILLDETAILREPOSITORY_H

#include "../models/billdetail.h"

// Writes BillDetail.txt (one line per product on a bill)
class BillDetailRepository
{
public:
    // Add one line at the end of the file
    bool save(const BillDetail& detail);
};

#endif