#include "BillDetailRepository.h"
#include "TextFormat.h"      // toText
#include "../config.h"       // BILL_DETAIL_FILE

#include <fstream>
using namespace std;

bool BillDetailRepository::save(const BillDetail& detail)
{
    ofstream file(BILL_DETAIL_FILE, ios::app);   // app = add at the end
    if (!file.is_open())
    {
        return false;
    }

    file << detail.billId << " "
         << detail.acronym << " "
         << toText(detail.unitPrice) << " "
         << toText(detail.quantity) << " "
         << toText(detail.discount)
         << "#" << "\n";

    return file.good();
}