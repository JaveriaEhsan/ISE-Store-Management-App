#include "BillRepository.h"   
#include "TextFormat.h"       // fixText and toText
#include "../config.h" 
#include <fstream>   
using namespace std;

// Writes one line to the bill file for each product on the bill
bool BillRepository::save(const Bill& bill)
{
    if (bill.lines.empty())                 
    {
        return false;
    }

    ofstream file(BILL_FILE, ios::app);   
    if (!file.is_open())                   
    {
        return false;
    }

    for (const BillLine& line : bill.lines)  // one file line per product
    {
        file << bill.billId << " "                    
             << fixText(bill.customerName) << " "
             << bill.orderId << " "
             << line.acronym << " "                   
             << toText(line.unitPrice) << " "
             << toText(line.discountPerUnit) << " "
             << toText(line.quantity) << " "
             << toText(bill.totalWithoutTax) << " "   
             << toText(bill.totalTax) << " "
             << toText(bill.totalDiscount) << " "
             << toText(bill.totalPayable) << " "
             << bill.status
             << "#" << "\n";
    }

    return file.good();   
}