#include "BillRepository.h"
#include "TextFormat.h"       
#include "../config.h"        

#include <fstream>
#include <sstream>
using namespace std;

// True if a bill with this id is already in Bill.txt
static bool billExists(const string& billId)
{
    ifstream in(BILL_FILE);
    string line;

    while (getline(in, line))
    {
        istringstream words(line);
        string firstWord;
        words >> firstWord;          // the bill id is the first field

        if (firstWord == billId)
        {
            return true;
        }
    }
    return false;
}

// Writes one line for the whole bill (14 fields)
bool BillRepository::save(const Bill& bill)
{
    if (bill.billId.empty())
    {
        return false;
    }

    if (billExists(bill.billId))     // same bill id must never be saved twice
    {
        return false;
    }

    ofstream file(BILL_FILE, ios::app);   // app = add at the end
    if (!file.is_open())
    {
        return false;
    }

    file << bill.billId << " "
         << fixText(bill.customerName) << " "
         << bill.orderId << " "
         << toText(bill.totalWithoutTax) << " "
         << toText(bill.totalTax) << " "
         << toText(bill.totalDiscount) << " "      // product discounts
         << toText(bill.totalPayable) << " "
         << toText(bill.amountReceived) << " "
         << toText(bill.balanceReturned) << " "
         << bill.status << " "
         << toText(bill.seasonalDiscount) << " "   // PKR amount
         << toText(bill.specialDiscount) << " "    // PKR amount
         << bill.date << " "
         << bill.time
         << "#" << "\n";

    return file.good();
}