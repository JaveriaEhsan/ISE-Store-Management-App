#include "paymentdialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>

using namespace std;

PaymentDialog::PaymentDialog(const string& customerName,
    const string& customerContact,
    const vector<BillItem>& items,
    double seasonalPercent,
    QWidget* parent)
    : QDialog(parent),
    customerName(customerName),
    customerContact(customerContact),
    items(items),
    seasonalPercent(seasonalPercent)
{
    setWindowTitle("Payment");

    modeBox = new QComboBox(this);
    modeBox->addItem(QString::fromStdString(PAY_CASH));
    modeBox->addItem(QString::fromStdString(PAY_CARD));
    modeBox->addItem(QString::fromStdString(PAY_WALLET));

    merchantEdit = new QLineEdit(this);
    merchantEdit->setPlaceholderText("for example ABL, JazzCash");

    specialSpin = new QDoubleSpinBox(this);
    specialSpin->setRange(0, MAX_SPECIAL_DISCOUNT_PERCENT);   // 0 to 8, so 9 cannot be typed
    specialSpin->setDecimals(2);
    specialSpin->setSingleStep(0.5);
    specialSpin->setSuffix(" %");

    receivedSpin = new QDoubleSpinBox(this);
    receivedSpin->setRange(0, 100000000);
    receivedSpin->setDecimals(2);

    payableLabel = new QLabel(this);
    balanceLabel = new QLabel(this);

    QFormLayout* form = new QFormLayout();
    form->addRow("Customer:", new QLabel(QString::fromStdString(customerName), this));
    form->addRow("Contact:", new QLabel(
        customerContact.empty() ? QString("-") : QString::fromStdString(customerContact), this));
    form->addRow("Payment mode:", modeBox);
    form->addRow("Merchant:", merchantEdit);
    form->addRow("Seasonal discount:", new QLabel(
        QString::number(seasonalPercent, 'g', 6) + " %", this));
    form->addRow("Special discount:", specialSpin);
    form->addRow("Payable:", payableLabel);
    form->addRow("Amount received:", receivedSpin);
    form->addRow("Balance:", balanceLabel);

    QDialogButtonBox* buttons =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);

    connect(modeBox, &QComboBox::currentTextChanged, this, &PaymentDialog::onModeChanged);
    connect(specialSpin, &QDoubleSpinBox::valueChanged, this, &PaymentDialog::onAmountsChanged);
    connect(receivedSpin, &QDoubleSpinBox::valueChanged, this, &PaymentDialog::onAmountsChanged);
    connect(buttons, &QDialogButtonBox::accepted, this, &PaymentDialog::onOk);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    onModeChanged();
    onAmountsChanged();
}

BillTotals PaymentDialog::currentTotals() const
{
    SeasonalDiscount seasonal(seasonalPercent);
    SpecialDiscount special(specialSpin->value());

    vector<const IDiscountRule*> rules;
    rules.push_back(&seasonal);
    rules.push_back(&special);

    return calculator.calculate(items, rules);   // the calculator does the maths
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

void PaymentDialog::onAmountsChanged()
{
    BillTotals t = currentTotals();
    payableLabel->setText(QString::number(t.totalPayable, 'f', 2));

    double balance = calculator.balance(receivedSpin->value(), t.totalPayable);
    balanceLabel->setText(QString::number(balance, 'f', 2));
}

void PaymentDialog::onOk()
{
    bool isCash = (modeBox->currentText().toStdString() == PAY_CASH);
    if (!isCash && merchantEdit->text().trimmed().isEmpty())
    {
        QMessageBox::warning(this, "Merchant required",
            "Merchant name is required for card and wallet payments.");
        return;
    }

    if (!SpecialDiscount::isValidPercent(specialSpin->value()))
    {
        QMessageBox::warning(this, "Special discount",
            "Special discount must be between 0 and 8 percent.");
        return;
    }

    if (!calculator.isEnough(receivedSpin->value(), currentTotals().totalPayable))
    {
        QMessageBox::warning(this, "Amount received",
            "Amount received must be at least the payable amount.");
        return;
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
    i.status = STATUS_PAID;                       // a bill is always paid; hold has its own button
    i.specialPercent = specialSpin->value();
    i.seasonalPercent = seasonalPercent;
    i.amountReceived = receivedSpin->value();
    return i;
}