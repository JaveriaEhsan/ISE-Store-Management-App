#ifndef BILLPDFEXPORTER_H
#define BILLPDFEXPORTER_H
#include <string>
#include <vector>
#include "models/bill.h"

class BillPdfExporter
{
public:
    // previous: the bill this one replaced (use previousBill to get it), or nullptr.
    // When given, the PDF shows the previous total and "Extra paid" or "Refund".
    bool exportBill(const Bill& bill, const std::string& filePath,
                    const std::string& note = "",
                    const Bill* previous = nullptr) const;
    // Builds the "REVISED / EDITED" line. Pass BillRepository::findByOrderId(bill.orderId):
    // the list must be in saving order. Only the bills just before and just after this
    // one are named. Cancelled bills are skipped. Returns "" if no note is needed.
    static std::string makeNote(const Bill& bill, const std::vector<Bill>& orderBills);

        // Returns the bill saved just before this one, or nullptr if there is none.
    // orderBills must come from BillRepository::findByOrderId(bill.orderId).
    // Keep that list alive while you use the returned pointer.
    static const Bill* previousBill(const Bill& bill, const std::vector<Bill>& orderBills);
};

#endif
