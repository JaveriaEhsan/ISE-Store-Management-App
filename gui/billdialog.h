#ifndef BILLDIALOG_H
#define BILLDIALOG_H

#include <QDialog>
#include "../services/OrderService.h"   


class BillDialog : public QDialog
{
    Q_OBJECT

public:
    BillDialog(const CheckoutResult& result, QWidget* parent = nullptr);
};

#endif