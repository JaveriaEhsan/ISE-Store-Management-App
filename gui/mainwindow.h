#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "data/IProductRepository.h"
#include "data/ICategoryRepository.h"
#include "data/ISeasonRepository.h"
#include "data/OrderRepository.h"
#include "data/OrderDetailRepository.h"
#include "data/BillRepository.h"
#include "data/BillDetailRepository.h"
#include "data/ApprovalRepository.h"
#include "services/ReportService.h"

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
    // references first: reportService is built from them
    IProductRepository& products;
    ICategoryRepository& categories;
    OrderRepository& orders;
    OrderDetailRepository& orderDetails;
    BillRepository& bills;
    BillDetailRepository& billDetails;
    ISeasonRepository& seasons;
    ApprovalRepository& approvals;
    ReportService reportService;        // must be declared after the references above

    QLabel* totalValue = nullptr;
    QLabel* lowValue = nullptr;
    QLabel* cashierBadge = nullptr;
    QLabel* managerBadge = nullptr;
    QPushButton* cashierButton = nullptr;
    QPushButton* managerButton = nullptr;
};

#endif