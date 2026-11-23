#include "BillPdfExporter.h"
#include <QPdfWriter>
#include <QPainter>
#include <QPageSize>
#include <QFont>
#include <QString>
#include <QStringList>

namespace {
QString money(double v) { return QString::number(v, 'f', 2); }
}

bool BillPdfExporter::exportBill(const Bill& bill, const std::string& filePath,
                                 const std::string& note) const
{
    QPdfWriter writer(QString::fromStdString(filePath));
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setResolution(100);               // 100 units = 1 inch, A4 is about 827 x 1169

    QPainter p(&writer);
    if (!p.isActive())
        return false;

    const int left = 50;
    const int right = writer.width() - 50;
    const int bottom = writer.height() - 60;
    const int rowH = 24;
    int y = 60;

    QFont normal("Helvetica", 10);
    QFont bold("Helvetica", 10, QFont::Bold);
    QFont title("Helvetica", 18, QFont::Bold);

    // column edges: item | barcode | qty | unit | discount | tax | total
    const int cx[] = { left, 230, 380, 440, 530, 610, 690, right };

    auto drawRow = [&](const QStringList& cells, const QFont& f) {
        p.setFont(f);
        for (int i = 0; i < 7; ++i) {
            QRect r(cx[i], y, cx[i + 1] - cx[i] - 4, rowH);
            int align = (i < 2) ? Qt::AlignLeft : Qt::AlignRight;
            p.drawText(r, align | Qt::AlignVCenter, cells.value(i));
        }
        y += rowH;
    };

    auto drawLabelValue = [&](const QString& label, const QString& value, const QFont& f) {
        p.setFont(f);
        p.drawText(QRect(430, y, 180, rowH), Qt::AlignLeft | Qt::AlignVCenter, label);
        p.drawText(QRect(610, y, right - 610, rowH), Qt::AlignRight | Qt::AlignVCenter, value);
        y += rowH;
    };

    auto header = [&]() {
        drawRow({ "Item", "Barcode", "Qty", "Unit", "Disc", "Tax", "Total" }, bold);
        p.drawLine(left, y, right, y);
        y += 4;
    };

    // title and bill info
    p.setFont(title);
    p.drawText(left, y + 30, "Bill");
    y += 50;
    p.setFont(normal);
    p.drawText(left, y, QString("Bill ID: %1").arg(QString::fromStdString(bill.billId)));
    p.drawText(430, y, QString("Date: %1  %2").arg(QString::fromStdString(bill.date),
                                                    QString::fromStdString(bill.time)));
    y += rowH;
    p.drawText(left, y, QString("Customer: %1").arg(QString::fromStdString(bill.customerName)));
    p.drawText(430, y, QString("Order ID: %1").arg(QString::fromStdString(bill.orderId)));
    y += rowH;
    p.drawText(left, y, QString("Status: %1").arg(QString::fromStdString(bill.status)));
    y += rowH;
    if (!note.empty()) {
        p.setFont(bold);
        p.drawText(left, y, QString::fromStdString(note));
        y += rowH;
    }
    y += 10;

    // lines
    header();
    for (const BillLine& l : bill.lines) {
        if (y + rowH > bottom) {            // simple page break
            writer.newPage();
            y = 60;
            header();
        }
        double lineTotal = (l.unitPrice - l.discountPerUnit + l.taxPerUnit) * l.quantity;
        drawRow({ QString::fromStdString(l.acronym), QString::fromStdString(l.barcode),
                  money(l.quantity), money(l.unitPrice), money(l.discount()),
                  money(l.taxPerUnit * l.quantity), money(lineTotal) }, normal);
    }
    p.drawLine(left, y, right, y);
    y += 10;

    // totals (keep the block together on one page)
    if (y + 9 * rowH > bottom) { writer.newPage(); y = 60; }
    drawLabelValue("Total without tax:", money(bill.totalWithoutTax), normal);
    drawLabelValue("Seasonal discount:", money(bill.seasonalDiscount), normal);
    drawLabelValue("Special discount:", money(bill.specialDiscount), normal);
    drawLabelValue("Total discount:", money(bill.totalDiscount), normal);
    drawLabelValue("Total tax:", money(bill.totalTax), normal);
    drawLabelValue("Total payable:", money(bill.totalPayable), bold);
    drawLabelValue("Amount received:", money(bill.amountReceived), normal);
    drawLabelValue("Balance returned:", money(bill.balanceReturned), normal);

    p.end();
    return true;
}
std::string BillPdfExporter::makeNote(const Bill& bill, const std::vector<Bill>& orderBills)
{
    if (bill.status == "edited") {
        for (const Bill& b : orderBills)
            if (b.billId != bill.billId && b.status != "edited")
                return "EDITED - replaced by bill " + b.billId;
        return "EDITED";
    }

    std::string older;
    int count = 0;
    for (const Bill& b : orderBills) {
        if (b.billId == bill.billId) continue;
        if (count > 0) older += ", ";
        older += b.billId;
        ++count;
    }
    if (count == 0) return "";
    return std::string("REVISED - replaces bill") + (count > 1 ? "s " : " ") + older;
}
