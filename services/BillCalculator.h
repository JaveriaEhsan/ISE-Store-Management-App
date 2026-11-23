#ifndef BILLCALCULATOR_H
#define BILLCALCULATOR_H

#include <string>
#include <vector>
#include "DiscountRules.h"

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
    double totalDiscount = 0;     // product discounts only
    double seasonalAmount = 0;    // PKR
    double specialAmount = 0;     // PKR
    double totalPayable = 0;
};

class BillCalculator
{
public:
    // The rules are optional, so calculate(items) still works
    BillTotals calculate(const std::vector<BillItem>& items,
        const std::vector<const IDiscountRule*>& rules
        = std::vector<const IDiscountRule*>()) const;

    double balance(double received, double payable) const;    // received - payable
    bool isEnough(double received, double payable) const;     // received >= payable
};

#endif