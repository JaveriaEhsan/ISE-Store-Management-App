#include "BillCalculator.h"
#include <cmath>

using namespace std;

static double round2(double value)
{
    return round(value * 100.0) / 100.0;
}

BillTotals BillCalculator::calculate(const vector<BillItem>& items,
    const vector<const IDiscountRule*>& rules) const
{
    BillTotals t;

    for (size_t i = 0; i < items.size(); i++)
    {
        t.totalWithoutTax += items[i].unitPrice * items[i].quantity;
        t.totalDiscount += items[i].discountPerUnit * items[i].quantity;
        t.totalTax += items[i].taxPerUnit * items[i].quantity;
    }
    t.totalWithoutTax = round2(t.totalWithoutTax);
    t.totalDiscount = round2(t.totalDiscount);
    t.totalTax = round2(t.totalTax);

    // seasonal and special discounts are taken from this amount
    double base = round2(t.totalWithoutTax - t.totalDiscount);

    double ruleTotal = 0;
    for (size_t i = 0; i < rules.size(); i++)
    {
        double a = round2(rules[i]->amount(base));
        if (rules[i]->name() == "seasonal") t.seasonalAmount += a;
        else if (rules[i]->name() == "special") t.specialAmount += a;
        ruleTotal += a;
    }

    t.totalPayable = round2(base - ruleTotal + t.totalTax);
    return t;
}

double BillCalculator::balance(double received, double payable) const
{
    return round2(received - payable);
}

bool BillCalculator::isEnough(double received, double payable) const
{
    return round2(received) >= round2(payable);
}