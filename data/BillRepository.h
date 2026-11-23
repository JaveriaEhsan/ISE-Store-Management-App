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

    // Every bill of one order (an order can have more than one bill)
    std::vector<Bill> findByOrderId(const std::string& orderId) const;

    // The bill of this order that is not "edited". Returns false if none.
    bool findCurrentByOrderId(const std::string& orderId, Bill& result) const;

    // Changes only the status of one bill. Returns false and writes nothing
    // if the id is missing or the status is not allowed.
    bool updateStatus(const std::string& billId, const std::string& status);
};

#endif