#include "BillRepository.h"   
#include "TextFormat.h"       // fixText and toText

#include <fstream>   

using namespace std;

bool BillRepository::save(const Bill& bill)
{
    ofstream file("data/Bill.txt", ios::app);   // app = add at end
    if (!file.is_open())                        
    {
        return false;
    }

    file << bill.billId << " "                    
         << fixText(bill.customerName) << " "     
         << bill.orderId << " "                   
         << bill.acronym << " "                   
         << toText(bill.unitPrice) << " "         
         << toText(bill.discountPerUnit) << " "   
         << toText(bill.quantity) << " "         
         << toText(bill.totalWithoutTax) << " "   
         << toText(bill.totalTax) << " "          
         << toText(bill.totalDiscount) << " "     
         << toText(bill.totalPayable) << " "      
         << bill.status                           
         << "#" << "\n";                          // end of line

    return true;
}