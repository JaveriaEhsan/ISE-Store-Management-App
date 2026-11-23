#include <QGuiApplication>
#include <QDir>
#include <iostream>
#include "services/BillPdfExporter.h"

static int failures = 0;

static void check(const std::string& name, const std::string& got, const std::string& expected)
{
    bool ok = (got == expected);
    if (!ok) ++failures;
    std::cout << (ok ? "PASS  " : "FAIL  ") << name;
    if (!ok) std::cout << "\n      got:      \"" << got << "\"\n      expected: \"" << expected << "\"";
    std::cout << std::endl;
}

static Bill mk(const std::string& id, const std::string& status)
{
    Bill b;
    b.billId = id;
    b.orderId = "1001";
    b.status = status;
    return b;
}

static Bill fullBill(const std::string& id, const std::string& status)
{
    Bill b = mk(id, status);
    b.customerName = "Test Customer";
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
    return b;
}

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);   // needed so Qt can load fonts

    // ---- note rule (no files needed) ----
    Bill old1 = mk("000000001", "edited");
    Bill old2 = mk("000000002", "edited");
    Bill cur  = mk("000000003", "paid");

    check("single bill, no note",
          BillPdfExporter::makeNote(cur, { cur }), "");
    check("edited bill points to current",
          BillPdfExporter::makeNote(old1, { old1, mk("000000002", "paid") }),
          "EDITED - replaced by bill 000000002");
    check("current replaces one old bill",
          BillPdfExporter::makeNote(mk("000000002", "paid"), { old1, mk("000000002", "paid") }),
          "REVISED - replaces bill 000000001");
    check("current replaces two old bills",
          BillPdfExporter::makeNote(cur, { old1, old2, cur }),
          "REVISED - replaces bills 000000001, 000000002");
    check("edited bill, no current found",
          BillPdfExporter::makeNote(old1, { old1 }), "EDITED");

    // ---- real PDFs with the notes ----
    BillPdfExporter exporter;
    Bill revised = fullBill("000000002", "paid");
    Bill edited  = fullBill("000000001", "edited");

    bool ok1 = exporter.exportBill(revised, "test_bill_revised.pdf",
                                   BillPdfExporter::makeNote(revised, { edited, revised }));
    bool ok2 = exporter.exportBill(edited, "test_bill_edited.pdf",
                                   BillPdfExporter::makeNote(edited, { edited, revised }));
    std::cout << (ok1 ? "OK: " : "FAILED: ") << QDir::current().absoluteFilePath("test_bill_revised.pdf").toStdString() << std::endl;
    std::cout << (ok2 ? "OK: " : "FAILED: ") << QDir::current().absoluteFilePath("test_bill_edited.pdf").toStdString() << std::endl;

    if (!ok1 || !ok2) ++failures;
    std::cout << (failures == 0 ? "ALL TESTS PASSED" : "SOME TESTS FAILED") << std::endl;
    return failures == 0 ? 0 : 1;
}
