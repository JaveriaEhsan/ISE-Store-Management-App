#ifndef BILLREPOSITORY_H
#define BILLREPOSITORY_H

#include "../models/bill.h"
#include <string>

// Writes Bill.txt (one line for each whole bill)
class BillRepository
{
public:
    // Adds one line at the end of Bill.txt.
    // Returns false and fills error if the bill cannot be saved.
    bool save(const Bill& bill, std::string& error);
};

#endif