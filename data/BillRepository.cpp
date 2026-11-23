#include "BillRepository.h"
#include "TextFormat.h"
#include "../config.h"

#include <fstream>
#include <sstream>
#include <vector>
using namespace std;

// Removes the last '#' and splits the line into words
static vector<string> splitFields(string line)
{
    if (!line.empty() && line.back() == '#')
    {
        line.pop_back();
    }

    istringstream in(line);
    vector<string> fields;
    string word;

    while (in >> word)
    {
        fields.push_back(word);
    }

    return fields;
}

// Turns one file line (14 fields) into a Bill. Returns false if the line is bad.
static bool parseBill(const string& line, Bill& b)
{
    vector<string> f = splitFields(line);

    if (f.size() != 14)
    {
        return false;
    }

    try
    {
        b.totalWithoutTax = stod(f[3]);
        b.totalTax = stod(f[4]);
        b.totalDiscount = stod(f[5]);
        b.totalPayable = stod(f[6]);
        b.amountReceived = stod(f[7]);
        b.balanceReturned = stod(f[8]);
        b.seasonalDiscount = stod(f[10]);
        b.specialDiscount = stod(f[11]);
    }
    catch (...)
    {
        return false;
    }

    b.billId = f[0];
    b.customerName = f[1];
    b.orderId = f[2];
    b.status = f[9];
    b.date = f[12];
    b.time = f[13];
    return true;
}

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

vector<Bill> BillRepository::getAll() const
{
    vector<Bill> result;
    ifstream file(BILL_FILE);
    string line;

    while (getline(file, line))
    {
        Bill b;
        if (parseBill(line, b))
        {
            result.push_back(b);
        }
    }

    return result;
}

bool BillRepository::findById(const string& billId, Bill& result) const
{
    vector<Bill> all = getAll();

    for (const Bill& b : all)
    {
        if (b.billId == billId)
        {
            result = b;
            return true;
        }
    }

    return false;
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