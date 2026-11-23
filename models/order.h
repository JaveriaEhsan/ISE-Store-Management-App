#ifndef ORDER_H
#define ORDER_H

#include <string>

const std::string STATUS_IN_PROGRESS = "in-progress";
const std::string STATUS_ON_HOLD     = "on-hold";
const std::string STATUS_PAID        = "paid";
const std::string STATUS_ARCHIVED    = "archived";
const std::string STATUS_EDITED      = "edited";    // Only bills use it

const std::string PAY_CASH   = "cash";
const std::string PAY_CARD   = "card";
const std::string PAY_WALLET = "wallet";

// One row of Order.txt
struct Order
{
    std::string orderId;              // 8 digits, the last two are the day of the order
    std::string date;                 // yyyy-mm-dd
    std::string time;                 // HH:mm
    std::string customerName;         // spaces are written as _ in the file
    std::string customerContact;      // optional, written as - when empty
    std::string status;               // one of the STATUS_ values
    double paymentAmount = 0;
    std::string paymentMode;          // one of the PAY_ values
    std::string merchant;             // for example ABL, JazzCash
};

#endif