#ifndef BILLPDFEXPORTER_H
#define BILLPDFEXPORTER_H
#include <string>
#include <vector>
#include "models/bill.h"

class BillPdfExporter
{
public:
    // Writes the bill to filePath as a PDF. Returns false if it could not be written.
    // note is an optional line under the header, see makeNote().
    bool exportBill(const Bill& bill, const std::string& filePath,
                    const std::string& note = "") const;

    // Builds the "REVISED / EDITED" line. Pass BillRepository::findByOrderId(bill.orderId):
    // the list must be in saving order. Only the bills just before and just after this
    // one are named. Cancelled bills are skipped. Returns "" if no note is needed.
    static std::string makeNote(const Bill& bill, const std::vector<Bill>& orderBills);
};

#endif
