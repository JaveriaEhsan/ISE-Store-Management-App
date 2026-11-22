#ifndef BILL_H
#define BILL_H
#include<vector>
#include <string>

// One product on the bill (becomes one row of BillDetail.txt)
struct BillLine
{
    std::string acronym;
    std::string barcode;
    double unitPrice = 0;         // without tax
    double discountPerUnit = 0;
    double taxPerUnit = 0;
    double quantity = 0;

    // total for BillDetail.txt
    double discount() const { 
        return discountPerUnit * quantity; 
    }   
};

// One whole bill for one customer(becomes one row of Bill.txt)
struct Bill
{
    std::string billId;          // 9 digits
    std::string customerName;
    std::string orderId;         // the order id from Order.txt
    std::vector<BillLine> lines; // all products on this bill
    double totalWithoutTax = 0;
    double totalTax = 0;
    double totalDiscount = 0;
    double totalPayable = 0;     // without tax - discount + tax
    std::string status;          // the order status from Order.txt

    double amountReceived = 0;
    double balanceReturned = 0;
    double seasonalDiscount = 0;   // PKR amount
    double specialDiscount = 0;    // PKR amount
    std::string date;              // yyyy-mm-dd
    std::string time;              // HH:mm
};

#endif