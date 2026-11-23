#ifndef BILLDIALOG_H
#define BILLDIALOG_H

#include <QDialog>
#include <QString>
#include "../services/OrderService.h"   // CheckoutResult

class QLabel;

// Shows a finished bill. Read only.
class BillDialog : public QDialog
{
    Q_OBJECT

public:
    BillDialog(const CheckoutResult& result, QWidget* parent = nullptr);

    // A note at the top, for example "EDITED - replaced by bill 000000007"
    void setNote(const QString& note);

private:
    QLabel* noteLabel;
};

#endif