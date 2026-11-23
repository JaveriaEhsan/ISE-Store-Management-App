#ifndef BILLDETAILREPOSITORY_H
#define BILLDETAILREPOSITORY_H

#include "../models/billdetail.h"
#include <vector>
#include <string>

// Reads and writes BillDetail.txt (one line per product on a bill)
class BillDetailRepository
{
public:
    // Adds one line at the end of the file
    bool save(const BillDetail& detail);

    // All product lines of one bill (to rebuild the bill for the PDF or reopen an order)
    std::vector<BillDetail> findByBillId(const std::string& billId) const;

    // Every line in the file (for the top-ten reports)
    std::vector<BillDetail> getAll() const;
};

#endif