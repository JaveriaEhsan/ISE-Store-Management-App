#ifndef BILLDIALOG_H
#define BILLDIALOG_H

#include <QDialog>
#include <QString>
#include <vector>
#include "../services/OrderService.h"  

class QLabel;


class BillDialog : public QDialog
{
    Q_OBJECT

public:
    BillDialog(const CheckoutResult& result, QWidget* parent = nullptr,
        const IProductRepository* products = nullptr);
    void setNote(const QString& note);

   
    void setOrderBills(const std::vector<Bill>& bills);

private slots:
    void savePdf();

private:
    QLabel* noteLabel;
    Bill shownBill;                  
    std::vector<Bill> orderBills;   
};

#endif