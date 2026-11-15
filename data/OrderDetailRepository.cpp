#include "OrderDetailRepository.h"   
#include "TextFormat.h"              // toText

#include <fstream>   

using namespace std;

bool OrderDetailRepository::save(const OrderDetail& detail)
{
    ofstream file("data/OrderDetail.txt", ios::app);   // app = add at end
    if (!file.is_open())                               
    {
        return false;
    }

    file << detail.orderId << " "        
         << detail.productId << " "       
         << detail.barcode << " "         
         << toText(detail.quantity)      // (2 decimals)
         << "#" << "\n";                  // end of line

    return true;
}