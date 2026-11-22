#ifndef ORDER_H
#define ORDER_H

#include <string>
using namespace std;

const string STATUS_IN_PROGRESS = "in-progress";
const string STATUS_ON_HOLD     = "on-hold";
const string STATUS_PAID        = "paid";
const string STATUS_ARCHIVED    = "archived";

const string PAY_CASH   = "cash";
const string PAY_CARD   = "card";
const string PAY_WALLET = "wallet";

// One row of Order.txt
struct Order
{
    string orderId;              // 8 digits, the last two are the day of the order
    string date;                 // dd-MM-yyyy
    string time;                 // HH:mm
    string customerName;         // spaces are written as _ in the file
    string customerContact;      // optional, written as - when empty
    string status;               // one of the STATUS_ values
    double paymentAmount = 0;
    string paymentMode;          // one of the PAY_ values
    string merchant;             // for example ABL, JazzCash
};

#endif