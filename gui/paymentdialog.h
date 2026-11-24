#ifndef PAYMENTDIALOG_H
#define PAYMENTDIALOG_H

#include <QDialog>
#include <string>
#include <vector>
#include "../services/OrderService.h"   // CheckoutInfo, BillCalculator, discount rules

class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;

// Asks for payment mode, merchant, special discount and amount received.
// After exec() == Accepted, call info() to get everything checkout needs.
class PaymentDialog : public QDialog
{
    Q_OBJECT

public:
    // items and seasonalPercent are only used to show the live payable and balance
    PaymentDialog(const std::string& customerName,
        const std::string& customerContact,
        const std::vector<BillItem>& items,
        double seasonalPercent,
        QWidget* parent = nullptr);

    // Call before exec() when a PAID order is being edited.
    // oldPayable = total of the bill that is replaced (OrderService::editedBillPayable).
    // The dialog then shows "Collect X" or "Refund X", and info().amountReceived
    // is only the EXTRA money taken now.
    void setEditMode(double oldPayable);

    CheckoutInfo info() const;

private slots:
    void onModeChanged();
    void onAmountsChanged();
    void onOk();

private:
    BillTotals currentTotals() const;

    std::string customerName;
    std::string customerContact;
    std::vector<BillItem> items;
    double seasonalPercent;
    BillCalculator calculator;

    bool editMode = false;
    double oldPayable = 0;

    QComboBox* modeBox;
    QLineEdit* merchantEdit;
    QDoubleSpinBox* specialSpin;
    QDoubleSpinBox* receivedSpin;
    QLabel* payableLabel;
    QLabel* balanceLabel;
    QLabel* receivedCaption;
    QLabel* balanceCaption;
    QLabel* oldCaption;
    QLabel* oldValue;
    QLabel* differenceCaption;
    QLabel* differenceValue;
};

#endif