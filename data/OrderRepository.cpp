#include "OrderRepository.h"

#include<fstream>
#include <iomanip>   // set precision
#include<sstream>
#include<string>

// empty text = "-", spaces = "_"

static string fixText(string text)
{
    if (text.empty())                
    {
        return "-";
    }

    for (int i = 0; i < text.size(); i++)  
    {
        if (text[i] == ' ')              // space found
        {
            text[i] = '_';               // replace with underscore
        }
    }
    return text;
}

bool OrderRepository::save(const Order& order)
{
    ofstream file("data/Order.txt", ios::app);   // app = add at end, keep old lines
    if (!file.is_open())                         
    {
        return false;
    }

    ostringstream amount;                                 // number to text
    amount << fixed << setprecision(2) << order.paymentAmount;   // 450.5 -> 450.50

    file << order.orderId << " "                
         << order.date << " "                   
         << order.time << " "                   
         << fixText(order.customerName) << " " 
         << fixText(order.customerContact) << " " 
         << order.status << " "                 
         << amount.str() << " "                
         << order.paymentMode << " "            
         << fixText(order.merchant)              // ( cash -> "-")
         << "#" << "\n";                         // end of line

    return true;                                 
}