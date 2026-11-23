#include "BillRepository.h"
#include "TextFormat.h"
#include "../config.h"
#include "../models/order.h" 

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

// Turns one file line (14 fields) into a Bill. Returns false if the line is bad.
static bool parseBill(const string& line, Bill& b)
{
    vector<string> f = splitLine(line);

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

// Turns a Bill into one file line (the only place the format is written)
static string makeLine(const Bill& b)
{
    ostringstream out;

    out << b.billId << " "
        << fixText(b.customerName) << " "
        << b.orderId << " "
        << toText(b.totalWithoutTax) << " "
        << toText(b.totalTax) << " "
        << toText(b.totalDiscount) << " "      // product discounts
        << toText(b.totalPayable) << " "
        << toText(b.amountReceived) << " "
        << toText(b.balanceReturned) << " "
        << b.status << " "
        << toText(b.seasonalDiscount) << " "   // PKR amount
        << toText(b.specialDiscount) << " "    // PKR amount
        << b.date << " "
        << b.time
        << "#";

    return out.str();
}

// True if a bill with this id is already in Bill.txt
static bool billExists(const string& billId)
{
    ifstream in(BILL_FILE);
    string line;

    while (getline(in, line))
    {
        vector<string> f = splitLine(line);

        if (!f.empty() && f[0] == billId)
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

// The allowed bill statuses
static bool validStatus(const string& status)
{
    return status == STATUS_PAID
        || status == STATUS_EDITED
        || status == STATUS_IN_PROGRESS
        || status == STATUS_ON_HOLD
        || status == STATUS_ARCHIVED;
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

vector<Bill> BillRepository::findByOrderId(const string& orderId) const
{
    vector<Bill> result;
    vector<Bill> all = getAll();

    for (const Bill& b : all)
    {
        if (b.orderId == orderId)
        {
            result.push_back(b);
        }
    }

    return result;
}

bool BillRepository::findCurrentByOrderId(const string& orderId, Bill& result) const
{
    vector<Bill> bills = findByOrderId(orderId);

    // the newest one that is not "edited" (the file is in saving order)
    for (size_t i = bills.size(); i > 0; i--)
    {
        if (bills[i - 1].status != STATUS_EDITED)
        {
            result = bills[i - 1];
            return true;
        }
    }

    return false;
}

bool BillRepository::updateStatus(const string& billId, const string& status)
{
    if (!validStatus(status))
    {
        return false;
    }

    vector<string> lines;
    bool found = false;

    {
        ifstream file(BILL_FILE);
        string line;

        while (getline(file, line))
        {
            if (line.empty())
            {
                continue;
            }

            Bill b;
            if (parseBill(line, b) && b.billId == billId)
            {
                b.status = status;
                line = makeLine(b);
                found = true;
            }

            lines.push_back(line);
        }
    }

    if (!found)
    {
        return false;
    }

    ofstream out(BILL_FILE, ios::trunc);
    if (!out.is_open())
    {
        return false;
    }

    for (const string& l : lines)
    {
        out << l << "\n";
    }

    return out.good();
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

    if (!validStatus(bill.status))
    {
        error = "The bill status is not allowed.";
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

    file << makeLine(bill) << "\n";

    if (!file.good())
    {
        error = "Writing to the bill file failed.";
        return false;
    }

    return true;
}