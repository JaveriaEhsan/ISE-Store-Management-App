#ifndef BILLREPOSITORY_H
#define BILLREPOSITORY_H

#include "../models/bill.h"

// Writes Bill.txt (one line for each whole bill)
class BillRepository
{
public:
    // Add one line at the end of Bill.txt.
    // Returns false if the bill id is empty or already used.
    bool save(const Bill& bill);
};

#endif