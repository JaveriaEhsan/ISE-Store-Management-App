#ifndef BILLPDFEXPORTER_H
#define BILLPDFEXPORTER_H
#include <string>
#include "models/bill.h"

class BillPdfExporter
{
public:
    // Writes the bill to filePath as a PDF. Returns false if it could not be written.
    // note is an optional line under the header (used in Part 2).
    bool exportBill(const Bill& bill, const std::string& filePath,
                    const std::string& note = "") const;
};

#endif