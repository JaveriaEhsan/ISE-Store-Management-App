#include "BillPdfExporter.h"
#include "models/order.h"
#include <QPdfWriter>
#include <QPainter>
#include <QPageSize>
#include <QFont>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QHash>
#include <QRegularExpression>

namespace {
QString money(double v) { return QString::number(v, 'f', 2); }

QString qty(double v)
{
    QString s = QString::number(v, 'f', 3);
    while (s.endsWith('0')) s.chop(1);
    if (s.endsWith('.')) s.chop(1);
    return s;
}

QString readableName(const std::string& name)
{
    QString s = QString::fromStdString(name);
    s.replace('_', ' ');
    return s.isEmpty() ? QString("-") : s;
}

QString percentText(double amount, double base)
{
    if (base <= 0 || amount <= 0) return "0 %";
    QString s = QString::number(amount / base * 100.0, 'f', 2);
    while (s.endsWith('0')) s.chop(1);
    if (s.endsWith('.')) s.chop(1);
    return s + " %";
}

// product.txt line: id category barcode acronym name type ...
QHash<QString, QString> loadProductNames(const QString& path)
{
    QHash<QString, QString> names;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return names;
    QTextStream in(&f);
    while (!in.atEnd()) {
        const QStringList t = in.readLine().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (t.size() < 5) continue;
        QString name = t[4];
        name.replace('_', ' ');
        names.insert(t[2], name);
    }
    return names;
}
}

bool BillPdfExporter::exportBill(const Bill& bill, const std::string& filePath,
                                 const std::string& note, const Bill* previous) const
{
    QPdfWriter writer(QString::fromStdString(filePath));
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setResolution(100);

    QPainter p(&writer);
    if (!p.isActive())
        return false;

    const int left = 50;
    const int right = writer.width() - 50;
    const int bottom = writer.height() - 60;
    const int rowH = 24;
    int y = 60;

    const QHash<QString, QString> productNames = loadProductNames("product.txt");

    QFont normal("Helvetica", 10);
    QFont bold("Helvetica", 10, QFont::Bold);
    QFont title("Helvetica", 18, QFont::Bold);

    // Barcode | Product | Qty | Price | Tax | Discount | Seasonal Disc | Total
    const int cx[] = { left, 150, 300, 350, 425, 495, 565, 655, right };

    auto drawRow = [&](const QStringList& cells, const QFont& f) {
        p.setFont(f);
        for (int i = 0; i < 8; ++i) {
            QRect r(cx[i], y, cx[i + 1] - cx[i] - 4, rowH);
            int align = (i < 2) ? Qt::AlignLeft : Qt::AlignRight;
            p.drawText(r, align | Qt::AlignVCenter,
                       p.fontMetrics().elidedText(cells.value(i),
                           (i < 2) ? Qt::ElideRight : Qt::ElideLeft, r.width()));
        }
        y += rowH;
    };                                              // <-- yeh closing zaroori hai

    auto drawLabelValue = [&](const QString& label, const QString& value, const QFont& f) {
        p.setFont(f);
        p.drawText(QRect(430, y, 240, rowH), Qt::AlignLeft | Qt::AlignVCenter, label);
        p.drawText(QRect(670, y, right - 670, rowH), Qt::AlignRight | Qt::AlignVCenter, value);
        y += rowH;
    };

    auto header = [&]() {
        drawRow({ "Barcode", "Product", "Qty", "Price", "Tax", "Discount", "Seasonal Disc", "Total" }, bold);
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
    p.drawText(left, y, QString("Customer: %1").arg(readableName(bill.customerName)));
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
    const double discountBase = bill.totalWithoutTax - bill.totalDiscount;   // seasonal and special are taken from this

    header();
    for (const BillLine& l : bill.lines) {
        if (y + rowH > bottom) {
            writer.newPage();
            y = 60;
            header();
        }
        const QString barcode = QString::fromStdString(l.barcode);
        const QString product = productNames.value(barcode, QString::fromStdString(l.acronym));

        const double lineBase = (l.unitPrice - l.discountPerUnit) * l.quantity;
        const double seasonalShare = discountBase > 0 ? bill.seasonalDiscount * lineBase / discountBase : 0.0;
        const double lineTotal = (l.unitPrice - l.discountPerUnit + l.taxPerUnit) * l.quantity;

        drawRow({ barcode, product, qty(l.quantity), money(l.unitPrice),
                  money(l.taxPerUnit * l.quantity), money(l.discount()),
                  money(seasonalShare), money(lineTotal) }, normal);
    }
    p.drawLine(left, y, right, y);
    y += 10;

    // totals (keep the block together on one page)
    if (y + 8 * rowH > bottom) { writer.newPage(); y = 60; }
    drawLabelValue("Total without tax:", money(bill.totalWithoutTax), normal);
    drawLabelValue("Seasonal discount (" + percentText(bill.seasonalDiscount, discountBase) + "):",
        money(bill.seasonalDiscount), normal);
    drawLabelValue("Special discount (" + percentText(bill.specialDiscount, discountBase) + "):",
        money(bill.specialDiscount), normal);
    drawLabelValue("Total discount:", money(bill.totalDiscount), normal);
    drawLabelValue("Total tax:", money(bill.totalTax), normal);
    drawLabelValue("Total payable:", money(bill.totalPayable), bold);
    drawLabelValue("Amount received:", money(bill.amountReceived), normal);
    drawLabelValue("Balance returned:", money(bill.balanceReturned), normal);

    if (previous) {
        const double diff = bill.totalPayable - previous->totalPayable;
        drawLabelValue("Previous bill total:", money(previous->totalPayable), normal);
        if (diff > 0.005)
            drawLabelValue("Extra paid:", money(diff), bold);
        else if (diff < -0.005)
            drawLabelValue("Refund:", money(-diff), bold);
        else
            drawLabelValue("Difference:", money(0), normal);
    }

    p.end();
    return true;
}
// orderBills must be in saving order, as returned by BillRepository::findByOrderId
// (it keeps file order). Only the bill saved just before / just after this one is named.
std::string BillPdfExporter::makeNote(const Bill& bill, const std::vector<Bill>& orderBills)
{
    if (bill.status == STATUS_CANCELLED)
        return "CANCELLED - not issued";                

    // issued bills only, still in saving order
    std::vector<const Bill*> issued;
    for (const Bill& b : orderBills)
        if (b.status != STATUS_CANCELLED)                
            issued.push_back(&b);

    size_t pos = issued.size();                          // size() means "not in the list"
    for (size_t i = 0; i < issued.size(); ++i)
        if (issued[i]->billId == bill.billId) { pos = i; break; }

    std::string note;
    if (pos > 0)                                         // a bill came before this one
        note = "REVISED - replaces bill " + issued[pos - 1]->billId;

    if (bill.status == STATUS_EDITED) {
        if (!note.empty()) note += "; ";
        if (pos + 1 < issued.size())                     // the bill saved right after
            note += "EDITED - replaced by bill " + issued[pos + 1]->billId;
        else
            note += "EDITED";
    }
    return note;
}
const Bill* BillPdfExporter::previousBill(const Bill& bill, const std::vector<Bill>& orderBills)
{
    const Bill* before = nullptr;                  // last real bill seen so far
    for (const Bill& b : orderBills) {
        if (b.status == STATUS_CANCELLED) continue; // skip cancelled bills
        if (b.billId == bill.billId) return before; // found ours, so return the one before
        before = &b;
    }
    return nullptr;                                // our bill was not in the list
}