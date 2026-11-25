#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "data/IProductRepository.h"
#include "data/ISeasonRepository.h"
#include "data/ICategoryRepository.h"
#include "data/OrderRepository.h"
#include "data/OrderDetailRepository.h"
#include "data/BillRepository.h"
#include "data/BillDetailRepository.h"
#include "data/ApprovalRepository.h"

class QLabel;
class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(IProductRepository& products, ICategoryRepository& categories,
               OrderRepository& orders, OrderDetailRepository& orderDetails, 
               BillRepository& bills, BillDetailRepository& billDetails, 
               ISeasonRepository& seasons, ApprovalRepository& approvals, 
               QWidget* parent = nullptr);
    
    void refreshStats();
    void refreshBadges();
private:
    IProductRepository& products;
    ICategoryRepository& categories;
    OrderRepository& orders;
    OrderDetailRepository& orderDetails;
    BillRepository& bills;
    BillDetailRepository& billDetails;
    ISeasonRepository& seasons;
    ApprovalRepository& approvals;
    
    QLabel* totalValue = nullptr;
    QLabel* lowValue = nullptr;
    QPushButton* cashierButton = nullptr;
    QPushButton* managerButton = nullptr;
    QLabel* cashierBadge = nullptr;
    QLabel* managerBadge = nullptr;
};

#endif