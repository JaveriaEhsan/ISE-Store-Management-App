#include "OrderRepository.h"
#include "TextFormat.h"
#include "../config.h"
#include <sstream>
#include <vector>
#include <fstream>

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

// Turns one file line into an Order. Returns false if the line is bad.
static bool parseOrder(const string& line, Order& o)
{
    vector<string> f = splitFields(line);

    if (f.size() != 9)
    {
        return false;
    }

    o.orderId = f[0];
    o.date = f[1];            // YYYY-MM-DD
    o.time = f[2];
    o.customerName = f[3];
    o.customerContact = f[4];
    o.status = f[5];

    try
    {
        o.paymentAmount = stod(f[6]);
    }
    catch (...)
    {
        o.paymentAmount = 0;
    }

    o.paymentMode = f[7];
    o.merchant = f[8];
    return true;
}

// Turns an Order into one file line (same format for save and update)
static string makeLine(const Order& order)
{
    ostringstream out;

    out << order.orderId << " "
        << order.date << " "
        << order.time << " "
        << fixText(order.customerName) << " "
        << fixText(order.customerContact) << " "
        << order.status << " "
        << toText(order.paymentAmount) << " "
        << order.paymentMode << " "
        << fixText(order.merchant)
        << "#";

    return out.str();
}

vector<Order> OrderRepository::getAll() const
{
    vector<Order> result;
    ifstream file(ORDER_FILE);
    string line;

    while (getline(file, line))
    {
        Order o;
        if (parseOrder(line, o))
        {
            result.push_back(o);
        }
    }

    return result;
}

bool OrderRepository::findById(const string& orderId, Order& result) const
{
    vector<Order> all = getAll();

    for (const Order& o : all)
    {
        if (o.orderId == orderId)
        {
            result = o;
            return true;
        }
    }

    return false;
}

// All orders that have this status (for example "on-hold")
vector<Order> OrderRepository::findByStatus(const string& status) const
{
    vector<Order> result;
    vector<Order> all = getAll();

    for (const Order& o : all)
    {
        if (o.status == status)
        {
            result.push_back(o);
        }
    }

    return result;
}

// Replaces the line that starts with this order id
bool OrderRepository::update(const Order& order)
{
    vector<string> lines;
    bool found = false;

    {
        ifstream file(ORDER_FILE);
        string line;

        while (getline(file, line))
        {
            if (line.empty())
            {
                continue;
            }

            if (line.rfind(order.orderId + " ", 0) == 0)
            {
                lines.push_back(makeLine(order));
                found = true;
            }
            else
            {
                lines.push_back(line);
            }
        }
    }

    if (!found)
    {
        return false;
    }

    ofstream out(ORDER_FILE, ios::trunc);
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

// Rewrites the file without the lines that start with this order id
bool OrderRepository::removeById(const string& orderId)
{
    vector<string> keep;

    {
        ifstream file(ORDER_FILE);
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

    ofstream out(ORDER_FILE, ios::trunc);
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

// Writes one order line at the end of the file
bool OrderRepository::save(const Order& order)
{
    ofstream file(ORDER_FILE, ios::app); 
    if (!file.is_open())
    {
        return false;
    }

    file << makeLine(order) << "\n";
    return file.good();
}