#include "ReportService.h"
#include "DateUtil.h"
#include <algorithm>
#include <iomanip>
#include <map>
#include <sstream>

using namespace std;

static string money(double v)
{
    ostringstream s;
    s << fixed << setprecision(2) << v;
    return s.str();
}

static string number(double v)
{
    ostringstream s;
    s << setprecision(10) << v;
    return s.str();
}

static string readable(string text)      // the files store spaces as _
{
    replace(text.begin(), text.end(), '_', ' ');
    return text;
}

static void addTotal(ReportTable& t, const string& label, double value)
{
    ReportTotal total;
    total.label = label;
    total.value = value;
    t.totals.push_back(total);
}

ReportService::ReportService(const OrderRepository& orders,
    const OrderDetailRepository& details,
    const BillRepository& bills,
    const IProductRepository& products)
    : orders(orders), details(details), bills(bills), products(products)
{
}

ReportTable ReportService::ordersTable(const string& title, const vector<Order>& list) const
{
    ReportTable t;
    t.title = title;
    t.headers = { "Order id", "Date", "Time", "Customer", "Status", "Amount" };

    double sum = 0;
    for (size_t i = 0; i < list.size(); i++)
    {
        t.rows.push_back({ list[i].orderId, list[i].date, list[i].time,
                           readable(list[i].customerName), list[i].status,
                           money(list[i].paymentAmount) });
        sum += list[i].paymentAmount;
    }
    addTotal(t, "Total amount", sum);
    return t;
}

ReportTable ReportService::incompleteOrders() const
{
    vector<Order> list;
    vector<Order> all = orders.getAll();
    for (size_t i = 0; i < all.size(); i++)
    {
        if (all[i].status != STATUS_PAID && all[i].status != STATUS_ARCHIVED)
            list.push_back(all[i]);
    }
    return ordersTable("Incomplete orders", list);
}

ReportTable ReportService::completedOrders() const
{
    vector<Order> list;
    vector<Order> all = orders.getAll();
    for (size_t i = 0; i < all.size(); i++)
    {
        if (all[i].status == STATUS_PAID || all[i].status == STATUS_ARCHIVED)
            list.push_back(all[i]);
    }
    return ordersTable("Completed orders", list);
}



bool ReportService::cashSales(const string& from, const string& to,
    ReportTable& table, string& error) const
{
    error.clear();
    string fromKey = dateKey(from);
    string toKey = dateKey(to);
    if (fromKey.empty() || toKey.empty())
    {
        error = "The dates are not valid, for example 2026-11-21.";
        return false;
    }
    if (fromKey > toKey)
    {
        error = "The start date is after the end date.";
        return false;
    }

    table = ReportTable();
    table.title = "Cash sales " + from + " to " + to;
    table.headers = { "Order id", "Date", "Time", "Customer", "Amount" };

    double sum = 0;
    vector<Order> all = orders.getAll();
    for (size_t i = 0; i < all.size(); i++)
    {
        const Order& o = all[i];
        string key = dateKey(o.date);
        bool wasPaid = (o.status == STATUS_PAID || o.status == STATUS_ARCHIVED);
        if (!wasPaid || o.paymentMode != PAY_CASH) continue;
        if (key.empty() || key < fromKey || key > toKey) continue;

        table.rows.push_back({ o.orderId, o.date, o.time,
                               readable(o.customerName), money(o.paymentAmount) });
        sum += o.paymentAmount;
    }
    addTotal(table, "Total cash sales", sum);
    return true;
}
void ReportService::periodRange(ReportPeriod period, string& from, string& to)
{
    to = todayDate();

    int day = 0, month = 0, year = 0;
    parseDate(to, day, month, year);

    if (period == PERIOD_DAILY)        from = to;
    else if (period == PERIOD_WEEKLY)  from = addDays(to, -6);
    else if (period == PERIOD_MONTHLY) from = makeDate(1, month, year);
    else                               from = makeDate(1, 1, year);
}

ReportTable ReportService::allSales(bool includeEdited) const
{
    ReportTable t;
    t.title = includeEdited ? "All sales (including edited bills)" : "All sales";
    t.headers = { "Bill id", "Order id", "Date", "Time", "Customer", "Status", "Payable",
                  "Product discount", "Seasonal discount", "Special discount" };

    double payable = 0, product = 0, seasonal = 0, special = 0;

    vector<Bill> all = bills.getAll();
    for (size_t i = 0; i < all.size(); i++)
    {
        const Bill& b = all[i];
        bool live = (b.status == STATUS_PAID);
        bool edited = (b.status == STATUS_EDITED);
        if (!live && !(includeEdited && edited)) continue;

        t.rows.push_back({ b.billId, b.orderId, b.date, b.time, readable(b.customerName), b.status,
                           money(b.totalPayable), money(b.totalDiscount),
                           money(b.seasonalDiscount), money(b.specialDiscount) });

        if (live)   // edited bills are shown on request, but never added to the totals
        {
            payable += b.totalPayable;
            product += b.totalDiscount;
            seasonal += b.seasonalDiscount;
            special += b.specialDiscount;
        }
    }

    addTotal(t, includeEdited ? "Total sales (live bills only)" : "Total sales", payable);
    addTotal(t, "Total product discount", product);
    addTotal(t, "Total seasonal discount", seasonal);
    addTotal(t, "Total special discount", special);
    addTotal(t, "Total discount (all kinds)", product + seasonal + special);
    return t;
}

ReportTable ReportService::topSoldItems() const
{
    ReportTable t;
    t.title = "Top ten most sold items";
    t.headers = { "Rank", "Barcode", "Product", "Sold", "Remaining stock" };

    // the order lines are used (not the bill lines), so an edited order is counted once
    map<string, double> sold;
    vector<Order> all = orders.getAll();
    for (size_t i = 0; i < all.size(); i++)
    {
        if (all[i].status != STATUS_PAID && all[i].status != STATUS_ARCHIVED) continue;

        vector<OrderDetail> d = details.findByOrderId(all[i].orderId);
        for (size_t j = 0; j < d.size(); j++)
        {
            sold[d[j].barcode] += d[j].quantity;
        }
    }

    vector<pair<string, double> > list(sold.begin(), sold.end());
    sort(list.begin(), list.end(),
        [](const pair<string, double>& a, const pair<string, double>& b)
        {
            if (a.second != b.second) return a.second > b.second;    // most sold first
            return a.first < b.first;
        });

    for (size_t i = 0; i < list.size() && i < 10; i++)
    {
        string acronym = "-";
        string remaining = "-";
        Product p;
        if (products.findByBarcode(list[i].first, p))
        {
            acronym = p.getAcronym();
            remaining = number(p.getQuantity());
        }
        t.rows.push_back({ to_string(i + 1), list[i].first, acronym,
                           number(list[i].second), remaining });
    }
    return t;
}

ReportTable ReportService::topStockItems() const
{
    ReportTable t;
    t.title = "Top ten items with the most stock left";
    t.headers = { "Rank", "Barcode", "Product", "Name", "Remaining stock" };

    vector<Product> list = products.getAll();
    sort(list.begin(), list.end(),
        [](const Product& a, const Product& b)
        {
            if (a.getQuantity() != b.getQuantity()) return a.getQuantity() > b.getQuantity();
            return a.getBarcode() < b.getBarcode();
        });

    for (size_t i = 0; i < list.size() && i < 10; i++)
    {
        t.rows.push_back({ to_string(i + 1), list[i].getBarcode(), list[i].getAcronym(),
                           readable(list[i].getName()), number(list[i].getQuantity()) });
    }
    return t;
}