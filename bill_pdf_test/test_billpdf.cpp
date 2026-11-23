#include <QGuiApplication>
#include <QDir>
#include <iostream>
#include "services/billpdfexporter.h"

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);     // needed so Qt can load fonts

    Bill b;
    b.billId = "000000001";
    b.customerName = "Test Customer";
    b.orderId = "1001";
    b.status = "paid";
    b.date = "23-11-2026";
    b.time = "17:30";
    b.lines.push_back({ "Milk", "100001", 100.0, 5.0, 17.0, 2 });
    b.lines.push_back({ "Bread", "200001", 80.0, 0.0, 14.4, 1 });
    b.totalWithoutTax = 280;
    b.totalDiscount = 10;
    b.seasonalDiscount = 6;
    b.specialDiscount = 4;
    b.totalTax = 48.4;
    b.totalPayable = 318.4;
    b.amountReceived = 400;
    b.balanceReturned = 81.6;

    BillPdfExporter exporter;
    bool ok = exporter.exportBill(b, "test_bill.pdf");
    std::cout << (ok ? "OK: " : "FAILED: ")
              << QDir::current().absoluteFilePath("test_bill.pdf").toStdString() << std::endl;
    return ok ? 0 : 1;
}
