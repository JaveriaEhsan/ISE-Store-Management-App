#ifndef BILLDIALOG_H
#define BILLDIALOG_H

#include <QDialog>
#include "../services/OrderService.h"   // CheckoutResult

// Shows a finished bill. Read only.
class BillDialog : public QDialog
{
    Q_OBJECT

public:
    BillDialog(const CheckoutResult& result, QWidget* parent = nullptr);
};

#endif