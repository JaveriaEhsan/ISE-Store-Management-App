#include "OrderDetailRepository.h"
#include "TextFormat.h"              // toText
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

// Turns one detail into a file line (same format for save and replaceForOrder)
static string makeLine(const OrderDetail& d)
{
    ostringstream out;

    out << d.orderId << " "
        << d.productId << " "
        << d.barcode << " "
        << toText(d.quantity)
        << "#";

    return out.str();
}

// All product lines of one order
vector<OrderDetail> OrderDetailRepository::findByOrderId(const string& orderId) const
{
    vector<OrderDetail> result;
    ifstream file(ORDER_DETAIL_FILE);
    string line;

    while (getline(file, line))
    {
        vector<string> f = splitFields(line);

        if (f.size() != 4 || f[0] != orderId)
        {
            continue;
        }

        OrderDetail d;
        d.orderId = f[0];
        d.productId = f[1];
        d.barcode = f[2];

        try
        {
            d.quantity = stod(f[3]);
        }
        catch (...)
        {
            continue;
        }

        result.push_back(d);
    }

    return result;
}

// Rewrites the file without the lines of this order
bool OrderDetailRepository::removeByOrderId(const string& orderId)
{
    vector<string> keep;

    {
        ifstream file(ORDER_DETAIL_FILE);
        string line;

        while (getline(file, line))
        {
            if (line.empty())
            {
                continue;
            }

            // starts with the id: drop it
            if (line.rfind(orderId + " ", 0) == 0)
            {
                continue;
            }

            keep.push_back(line);
        }
    }

    ofstream out(ORDER_DETAIL_FILE, ios::trunc);
    if (!out.is_open())
    {
        return false;
    }

    for (const string& l : keep)
    {
        out << l << "\n";
    }

    return out.good();
}

// Swaps all lines of one order for the new ones
bool OrderDetailRepository::replaceForOrder(const string& orderId,
                                            const vector<OrderDetail>& details)
{
    vector<string> lines;

    // 1. Keep every line that belongs to other orders
    {
        ifstream file(ORDER_DETAIL_FILE);
        string line;

        while (getline(file, line))
        {
            if (line.empty() || line.rfind(orderId + " ", 0) == 0)
            {
                continue;
            }

            lines.push_back(line);
        }
    }

    // 2. Add the new lines of this order
    for (const OrderDetail& d : details)
    {
        lines.push_back(makeLine(d));
    }

    // 3. Write everything back in one go
    ofstream out(ORDER_DETAIL_FILE, ios::trunc);
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

// Writes one product line at the end of the file
bool OrderDetailRepository::save(const OrderDetail& detail)
{
    ofstream file(ORDER_DETAIL_FILE, ios::app);   // app = add at end
    if (!file.is_open())
    {
        return false;
    }

    file << makeLine(detail) << "\n";
    return file.good();
}