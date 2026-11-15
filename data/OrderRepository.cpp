#include "OrderRepository.h"
#include "TextFormat.h" // fixText and toText

#include <fstream>

using namespace std;

bool OrderRepository::save(const Order &order)
{
    ofstream file("Order.txt", ios::app); // app = add at end
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