#include "BillDetailRepository.h"
#include "TextFormat.h"
#include "../config.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

// Turns one file line into a BillDetail. Returns false if the line is bad.
static bool parseLine(const string& line, BillDetail& d)
{
    vector<string> f = splitLine(line);

    if (f.size() != 5)
    {
        return false;
    }

    try
    {
        d.unitPrice = stod(f[2]);
        d.quantity = stod(f[3]);
        d.discount = stod(f[4]);
    }
    catch (...)
    {
        return false;
    }

    d.billId = f[0];
    d.acronym = f[1];
    return true;
}

// Turns a BillDetail into one file line (same format for every write)
static string makeLine(const BillDetail& d)
{
    ostringstream out;

    out << d.billId << " "
        << d.acronym << " "
        << toText(d.unitPrice) << " "
        << toText(d.quantity) << " "
        << toText(d.discount)
        << "#";

    return out.str();
}

vector<BillDetail> BillDetailRepository::getAll() const
{
    vector<BillDetail> result;
    ifstream file(BILL_DETAIL_FILE);
    string line;

    while (getline(file, line))
    {
        BillDetail d;
        if (parseLine(line, d))
        {
            result.push_back(d);
        }
    }

    return result;
}

vector<BillDetail> BillDetailRepository::findByBillId(const string& billId) const
{
    vector<BillDetail> result;
    vector<BillDetail> all = getAll();

    for (const BillDetail& d : all)
    {
        if (d.billId == billId)
        {
            result.push_back(d);
        }
    }

    return result;
}

bool BillDetailRepository::save(const BillDetail& detail)
{
    ofstream file(BILL_DETAIL_FILE, ios::app);   // app = add at the end
    if (!file.is_open())
    {
        return false;
    }

    file << makeLine(detail) << "\n";
    return file.good();
}