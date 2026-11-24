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

static QString money(double value)
{
    return QString::number(value, 'f', 2);
}

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
    receivedCaption = new QLabel("Amount received:", this);
    balanceCaption = new QLabel("Balance:", this);

    // these four are only shown when a paid order is edited
    oldCaption = new QLabel("Old bill total:", this);
    oldValue = new QLabel(this);
    differenceCaption = new QLabel("Difference:", this);
    differenceValue = new QLabel(this);

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
    form->addRow(oldCaption, oldValue);
    form->addRow(differenceCaption, differenceValue);
    form->addRow(receivedCaption, receivedSpin);
    form->addRow(balanceCaption, balanceLabel);

    oldCaption->hide();
    oldValue->hide();
    differenceCaption->hide();
    differenceValue->hide();

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

void PaymentDialog::setEditMode(double oldPayableTotal)
{
    editMode = true;
    oldPayable = oldPayableTotal;

    setWindowTitle("Payment - edit paid order");
    oldValue->setText(money(oldPayable));
    oldCaption->show();
    oldValue->show();
    differenceCaption->show();
    differenceValue->show();

    receivedCaption->setText("Extra money received:");
    balanceCaption->setText("Balance returned:");

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
    payableLabel->setText(money(t.totalPayable));

    if (!editMode)
    {
        balanceLabel->setText(money(calculator.balance(receivedSpin->value(), t.totalPayable)));
        return;
    }

    double difference = calculator.difference(t.totalPayable, oldPayable);
    if (difference > 0)       differenceValue->setText("Collect " + money(difference));
    else if (difference < 0)  differenceValue->setText("Refund " + money(qAbs(difference)));
    else                      differenceValue->setText("Nothing to pay or refund");

    // extra money is only taken when the new total is higher than the old one
    receivedSpin->setEnabled(difference > 0);
    if (difference <= 0 && receivedSpin->value() != 0)
    {
        receivedSpin->setValue(0);
    }

    // the same figures the new bill will store
    double paid = calculator.paidAfterEdit(oldPayable, receivedSpin->value());
    balanceLabel->setText(money(calculator.balance(paid, t.totalPayable)));
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

    BillTotals t = currentTotals();

    if (editMode)
    {
        double difference = calculator.difference(t.totalPayable, oldPayable);
        if (difference > 0 && !calculator.isEnough(receivedSpin->value(), difference))
        {
            QMessageBox::warning(this, "Extra money",
                "The extra money must cover the difference.");
            return;
        }
    }
    else if (!calculator.isEnough(receivedSpin->value(), t.totalPayable))
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
    i.amountReceived = receivedSpin->value();     // edit mode: only the extra money
    return i;
}