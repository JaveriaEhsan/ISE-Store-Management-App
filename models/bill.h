#ifndef BILL_H
#define BILL_H
#include<vector>
#include <string>
using namespace std;

// One product on the bill (becomes one row of Bill.txt)
struct BillLine
{
    string acronym;
    double unitPrice = 0;         // without tax
    double discountPerUnit = 0;
    double quantity = 0;
};

// One whole bill for one customer
struct Bill
{
    string billId;               // 9 digits
    string customerName;
    string orderId;              // the order id from Order.txt
    std::vector<BillLine> lines; // all products on this bill
    double totalWithoutTax = 0;
    double totalTax = 0;
    double totalDiscount = 0;
    double totalPayable = 0;     // without tax - discount + tax
    string status;               // the order status from Order.txt
};

#endif