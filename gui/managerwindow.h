#ifndef MANAGERWINDOW_H
#define MANAGERWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include "../data/IProductRepository.h"

class QWidget;
class QPushButton;
class QLineEdit;
class QLabel;
class QFrame;
class QHBoxLayout;
class QStackedWidget;

class ManagerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ManagerWindow(QWidget *parent = nullptr);
    void setRepository(IProductRepository *repo);

private slots:
    void generateBarcode();        // Regenerate Barcode (existing product)
    void generateAllBarcodes();
    void productNumberChanged(const QString &text);

    void showDashboard();
    void showBarcodes();
    void showInventory();
    void showProductScan();
    void showAddProduct();         // Add New Product

private:
    void setupUI();
    void createSidebar();
    void createMainContent();

    QWidget *createDashboardPage();
    QWidget *createBarcodePage();

    void syncSidebarSelection();
    void resetBarcodePreview();

    IProductRepository *repository;

    QWidget *sidebar;
    QWidget *mainContent;
    QHBoxLayout *bodyLayout;
    QStackedWidget *pages;

    QWidget *dashboardPage;
    QWidget *barcodePage;

    QPushButton *dashboardButton;
    QPushButton *barcodesButton;
    QPushButton *inventoryButton;
    QPushButton *productScanButton;

    QLineEdit *productNumberInput;
    QPushButton *generateBarcodeButton;
    QPushButton *generateAllBarcodesButton;

    QFrame *productsCard;
    QFrame *barcodeCard;
    QLabel *barcodeImage;
    QLabel *barcodeDisplay;
};

#endif