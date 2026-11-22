#include "BillCalculator.h"
using namespace std;

BillTotals BillCalculator::calculate(const vector<BillItem>& items) const
{
    BillTotals t;
    for (size_t i = 0; i < items.size(); i++)
    {
        t.totalWithoutTax += items[i].unitPrice * items[i].quantity;
        t.totalTax += items[i].taxPerUnit * items[i].quantity;
        t.totalDiscount += items[i].discountPerUnit * items[i].quantity;
    }
    t.totalPayable = t.totalWithoutTax - t.totalDiscount + t.totalTax;
    return t;
}