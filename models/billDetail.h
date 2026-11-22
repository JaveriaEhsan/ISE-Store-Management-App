#ifndef BILLDETAIL_H
#define BILLDETAIL_H

#include <string>

// One row of BillDetail.txt
struct BillDetail
{
    std::string billId;
    std::string acronym;
    double unitPrice = 0;
    double quantity = 0;
    double discount = 0;     // total discount on this product
};

#endif