#include "OrderRepository.h"
#include "TextFormat.h"
#include "../config.h"
#include <sstream>
#include <vector>
#include <fstream>

using namespace std;

// Reads one line of the file into an Order.
// Returns false if the line is damaged.
static bool parseOrder(const string& line, Order& o)
{
    vector<string> f = splitLine(line);

    // a good line has exactly 9 fields
    if (f.size() != 9)
    {
        return false;
    }

    try
    {
        o.paymentAmount = stod(f[6]);
    }
    catch (...)
    {
        return false;       // the amount is not a number, so the line is damaged
    }

    o.orderId = f[0];
    o.date = f[1];          // the date format is set in DateUtil
    o.time = f[2];
    o.customerName = f[3];
    o.customerContact = f[4];
    o.status = f[5];
    o.paymentMode = f[7];
    o.merchant = f[8];
    return true;
}

// Turns an Order into one line of text.
// save and update both use this, so the format is the same everywhere.
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
        << fixText(order.paymentMode) << " "     // an empty mode is written as -
        << fixText(order.merchant)
        << "#";

    return out.str();
}

// True if the text is empty or has a space.
// Such a value would move the other fields in the line.
static bool badField(const string& text)
{
    return text.empty() || text.find(' ') != string::npos;
}

// True if the status is one of the four allowed ones
static bool validStatus(const string& status)
{
    return status == STATUS_IN_PROGRESS
        || status == STATUS_ON_HOLD
        || status == STATUS_PAID
        || status == STATUS_ARCHIVED;
}

// True if the payment mode is cash, card or wallet
static bool validPaymentMode(const string& mode)
{
    return mode == PAY_CASH || mode == PAY_CARD || mode == PAY_WALLET;
}

// Checks an order before it is written.
// Returns an empty text if it is fine, otherwise the reason.
static string checkOrder(const Order& order)
{
    if (badField(order.orderId))
    {
        return "The order id must not be empty or contain spaces.";
    }

    if (badField(order.date) || badField(order.time))
    {
        return "The order date and time must not be empty or contain spaces.";
    }

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

// True if the file already has an order with this id
static bool orderExists(const string& orderId)
{
    ifstream in(ORDER_FILE);
    string line;

    while (getline(in, line))
    {
        vector<string> f = splitLine(line);

        // the order id is the first field of the line
        if (!f.empty() && f[0] == orderId)
        {
            return true;
        }
    }

    return false;
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
            result.push_back(o);    // damaged lines are skipped
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

    // read the whole file, and swap the line of this order
    {
        ifstream file(ORDER_FILE);
        string line;

        while (getline(file, line))
        {
            if (line.empty())
            {
                continue;
            }

            // the line starts with this order id
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

    // write everything back in one go
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

    // keep every line except the one that starts with this order id
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

    // write the kept lines back
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

    // two orders must never share the same id
    if (orderExists(order.orderId))
    {
        error = "An order with this id already exists.";
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