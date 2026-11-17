#ifndef PAYMENTDIALOG_H
#define PAYMENTDIALOG_H

#include <QDialog>
#include <string>
#include "../services/OrderService.h"   // gives CheckoutInfo

class QComboBox;
class QLineEdit;

// Asks for payment mode, merchant and status. After exec() == Accepted,
// call info() to get everything checkout needs.
class PaymentDialog : public QDialog
{
    Q_OBJECT

public:
    PaymentDialog(const std::string& customerName,
        const std::string& customerContact,
        QWidget* parent = nullptr);

    CheckoutInfo info() const;

private slots:
    void onModeChanged();
    void onOk();

private:
    std::string customerName;
    std::string customerContact;

    QComboBox* modeBox;
    QLineEdit* merchantEdit;
    QComboBox* statusBox;
};

#endif