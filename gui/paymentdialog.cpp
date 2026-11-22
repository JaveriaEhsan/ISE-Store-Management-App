#include "paymentdialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>

using namespace std;

PaymentDialog::PaymentDialog(const string& customerName,
    const string& customerContact,
    QWidget* parent)
    : QDialog(parent),
    customerName(customerName),
    customerContact(customerContact)
{
    setWindowTitle("Payment");

    modeBox = new QComboBox(this);
    modeBox->addItem(QString::fromStdString(PAY_CASH));
    modeBox->addItem(QString::fromStdString(PAY_CARD));
    modeBox->addItem(QString::fromStdString(PAY_WALLET));

    merchantEdit = new QLineEdit(this);
    merchantEdit->setPlaceholderText("for example ABL, JazzCash");

    statusBox = new QComboBox(this);
    statusBox->addItem(QString::fromStdString(STATUS_PAID));
    statusBox->addItem(QString::fromStdString(STATUS_ON_HOLD));
    statusBox->addItem(QString::fromStdString(STATUS_IN_PROGRESS));
    statusBox->addItem(QString::fromStdString(STATUS_ARCHIVED));
    QFormLayout* form = new QFormLayout();
    form->addRow("Customer:", new QLabel(QString::fromStdString(customerName), this));
    form->addRow("Contact:", new QLabel(
        customerContact.empty() ? QString("-") : QString::fromStdString(customerContact), this));
    form->addRow("Payment mode:", modeBox);
    form->addRow("Merchant:", merchantEdit);
    form->addRow("Status:", statusBox);

    QDialogButtonBox* buttons =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);

    connect(modeBox, &QComboBox::currentTextChanged, this, &PaymentDialog::onModeChanged);
    connect(buttons, &QDialogButtonBox::accepted, this, &PaymentDialog::onOk);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    onModeChanged();   // set the starting state (cash -> merchant disabled)
}

void PaymentDialog::onModeChanged()
{
    bool isCash = (modeBox->currentText().toStdString() == PAY_CASH);
    merchantEdit->setEnabled(!isCash);
    if (isCash)
    {
        merchantEdit->clear();
    }
}

void PaymentDialog::onOk()
{
    bool isCash = (modeBox->currentText().toStdString() == PAY_CASH);
    if (!isCash && merchantEdit->text().trimmed().isEmpty())
    {
        QMessageBox::warning(this, "Merchant required",
            "Merchant name is required for card and wallet payments.");
        return;   // stay open
    }
    accept();
}

CheckoutInfo PaymentDialog::info() const
{
    CheckoutInfo i;
    i.customerName = customerName;
    i.customerContact = customerContact;
    i.paymentMode = modeBox->currentText().toStdString();
    i.merchant = merchantEdit->text().trimmed().toStdString();
    i.status = statusBox->currentText().toStdString();
    return i;
}