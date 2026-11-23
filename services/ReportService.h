#ifndef REPORTSERVICE_H
#define REPORTSERVICE_H

#include <string>
#include <vector>
#include "../models/order.h"
#include "../models/orderDetail.h"
#include "../data/OrderRepository.h"
#include "../data/OrderDetailRepository.h"
#include "../data/BillRepository.h"
#include "../data/IProductRepository.h"

struct ReportTotal
{
    std::string label;
    double value = 0;
};

// Every report is the same plain table, so ReportsWindow shows any report the same way.
struct ReportTable
{
    std::string title;
    std::vector<std::string> headers;
    std::vector<std::vector<std::string> > rows;
    std::vector<ReportTotal> totals;       // empty when the report has no totals
};

enum ReportPeriod { PERIOD_DAILY, PERIOD_WEEKLY, PERIOD_MONTHLY, PERIOD_YEARLY };

class ReportService
{
private:
    const OrderRepository& orders;
    const OrderDetailRepository& details;
    const BillRepository& bills;
    const IProductRepository& products;

    ReportTable ordersTable(const std::string& title, const std::vector<Order>& list) const;

public:
    ReportService(const OrderRepository& orders,
        const OrderDetailRepository& details,
        const BillRepository& bills,
        const IProductRepository& products);

    // orders that are not paid or archived, with the total amount
    ReportTable incompleteOrders() const;
    // paid or archived orders, with the total amount
    ReportTable completedOrders() const;

    // paid cash orders from "from" to "to" (dd-MM-yyyy, both included)
    bool cashSales(const std::string& from, const std::string& to,
        ReportTable& table, std::string& error) const;

    // fills from/to for a period ending today:
    // daily = today, weekly = last 7 days, monthly = this month, yearly = this year
    static void periodRange(ReportPeriod period, std::string& from, std::string& to);

    // paid bills with payable amount and discounts (the latest bill of each order)
    ReportTable allSales() const;

    ReportTable topSoldItems() const;     // ten most sold, with remaining stock
    ReportTable topStockItems() const;    // ten with the most stock left
};

#endif