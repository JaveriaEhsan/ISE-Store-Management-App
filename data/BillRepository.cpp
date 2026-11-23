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

// True if the text is empty or has a space (it would shift the columns)
static bool badField(const string& text)
{
    return text.empty() || text.find(' ') != string::npos;
}

// Writes one line for the whole bill (14 fields)
bool BillRepository::save(const Bill& bill, string& error)
{
    error.clear();

    if (badField(bill.billId))
    {
        error = "The bill id must not be empty or contain spaces.";
        return false;
    }

    if (badField(bill.date) || badField(bill.time))
    {
        error = "The bill date and time must not be empty or contain spaces.";
        return false;
    }

    if (billExists(bill.billId))
    {
        error = "A bill with this id already exists.";
        return false;
    }

    ofstream file(BILL_FILE, ios::app);
    if (!file.is_open())
    {
        error = "The bill file could not be opened.";
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

    if (!file.good())
    {
        error = "Writing to the bill file failed.";
        return false;
    }

    return true;
}