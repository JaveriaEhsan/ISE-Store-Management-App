#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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

private:
    // references: the window uses the objects that main.cpp created
    IProductRepository& products;
    OrderRepository& orders;
    OrderDetailRepository& orderDetails;
    BillRepository& bills;
};

#endif