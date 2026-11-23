#ifndef BILLREPOSITORY_H
#define BILLREPOSITORY_H

#include "../models/bill.h"
#include <string>
#include <vector>

// Reads and writes Bill.txt (one line for each whole bill)
class BillRepository
{
public:
    // Adds one line at the end of Bill.txt.
    // Returns false and fills error if the bill cannot be saved.
    bool save(const Bill& bill, std::string& error);

    // Every bill in the file (totals only, lines come from BillDetailRepository)
    std::vector<Bill> getAll() const;

    // Finds one bill by its id. Returns false if not found.
    bool findById(const std::string& billId, Bill& result) const;
};

#endif