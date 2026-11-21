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

// All orders that have this status (for example "on-hold")
vector<Order> OrderRepository::findByStatus(const string& status) const
{
    vector<Order> result;
    ifstream file(ORDER_FILE);
    string line;

    while (getline(file, line))
    {
        vector<string> f = splitFields(line);

        if (f.size() != 9 || f[5] != status)
        {
            continue;
        }

        Order o;
        o.orderId = f[0];
        o.date = f[1];
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
        result.push_back(o);
    }

    return result;
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

bool OrderRepository::save(const Order& order)
{
    ofstream file(ORDER_FILE, ios::app); // app = add at end
    if (!file.is_open())
    {
        return false;
    }

    file << order.orderId << " "
         << order.date << " "
         << order.time << " "
         << fixText(order.customerName) << " "
         << fixText(order.customerContact) << " "
         << order.status << " "
         << toText(order.paymentAmount) << " "
         << order.paymentMode << " "
         << fixText(order.merchant) // (cash -> "-")
         << "#" << "\n";            // end of line

    return true;
}