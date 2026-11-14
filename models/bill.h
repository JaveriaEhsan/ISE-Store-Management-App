#ifndef BILL_H
#define BILL_H

#include <string>
using namespace std;

// One row of Bill.txt
struct Bill
{
    string billId;               // 9 digits
    string customerName;
    string orderId;              // the order id from Order.txt
    string acronym;              // product acronym
    double unitPrice = 0;        // without tax
    double discountPerUnit = 0;
    double quantity = 0;
    double totalWithoutTax = 0;
    double totalTax = 0;
    double totalDiscount = 0;
    double totalPayable = 0;     // without tax - discount + tax
    string status;               // the order status from Order.txt
};

#endif