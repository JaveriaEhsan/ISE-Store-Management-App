#include "OrderRepository.h"
#include "TextFormat.h"
#include "../config.h"
#include <sstream>
#include <vector>
#include <fstream>

using namespace std;

// Turns one file line into an Order. Returns false if the line is bad.
static bool parseOrder(const string& line, Order& o)
{
    vector<string> f = splitLine(line);

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
        << fixText(order.paymentMode) << " "     // empty mode is written as -
        << fixText(order.merchant)
        << "#";

    return out.str();
}

// The four allowed order statuses
static bool validStatus(const string& status)
{
    return status == STATUS_IN_PROGRESS
        || status == STATUS_ON_HOLD
        || status == STATUS_PAID
        || status == STATUS_ARCHIVED;
}

// The allowed payment modes
static bool validPaymentMode(const string& mode)
{
    return mode == PAY_CASH || mode == PAY_CARD || mode == PAY_WALLET;
}

// Returns "" if the order is fine, otherwise the reason
static string checkOrder(const Order& order)
{
    if (!validStatus(order.status))
    {
        return "The status must be in-progress, on-hold, paid or archived.";
    }

    // an order that is not paid yet may have no payment mode
    if (order.status == STATUS_PAID && !validPaymentMode(order.paymentMode))
    {
        return "The payment mode must be cash, card or wallet.";
    }

    return "";
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

bool OrderRepository::update(const Order& order, string& error)
{
    error = checkOrder(order);
    if (!error.empty())
    {
        return false;
    }

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
        error = "No order with this id.";
        return false;
    }

    ofstream out(ORDER_FILE, ios::trunc);
    if (!out.is_open())
    {
        error = "The order file could not be opened.";
        return false;
    }

    for (const string& l : lines)
    {
        out << l << "\n";
    }

    if (!out.good())
    {
        error = "Writing to the order file failed.";
        return false;
    }

    return true;
}

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

bool OrderRepository::save(const Order& order, string& error)
{
    error = checkOrder(order);
    if (!error.empty())
    {
        return false;
    }

    ofstream file(ORDER_FILE, ios::app);
    if (!file.is_open())
    {
        error = "The order file could not be opened.";
        return false;
    }

    file << makeLine(order) << "\n";

    if (!file.good())
    {
        error = "Writing to the order file failed.";
        return false;
    }

    return true;
}