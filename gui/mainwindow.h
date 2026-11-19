#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include "data/IProductRepository.h"
#include "data/OrderRepository.h"
#include "data/OrderDetailRepository.h"
#include "data/BillRepository.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(IProductRepository& products, OrderRepository& orders,
               OrderDetailRepository& orderDetails, BillRepository& bills,
               QWidget* parent = nullptr);
    
    void refreshStats();
private:
    IProductRepository& products;
    OrderRepository& orders;
    OrderDetailRepository& orderDetails;
    BillRepository& bills;
    
    QLabel* totalValue = nullptr;
    QLabel* lowValue = nullptr;
};

#endif