#ifndef BILLCALCULATOR_H
#define BILLCALCULATOR_H

#include <string>
#include <vector>

// One line of the current order
struct BillItem
{
    std::string productId;
    std::string barcode;
    std::string acronym;
    double unitPrice = 0;         // without tax
    double taxPerUnit = 0;
    double discountPerUnit = 0;
    double quantity = 0;
};

struct BillTotals
{
    double totalWithoutTax = 0;
    double totalTax = 0;
    double totalDiscount = 0;
    double totalPayable = 0;
};

class BillCalculator
{
public:
    BillTotals calculate(const std::vector<BillItem>& items) const;
};

#endif