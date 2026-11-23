#ifndef MANAGERWINDOW_H
#define MANAGERWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include "../data/IProductRepository.h"
#include "../data/ICategoryRepository.h"

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
    explicit ManagerWindow(QWidget* parent = nullptr);
    void setRepository(IProductRepository* repo);
    void setCategoryRepository(ICategoryRepository* repo);

private slots:
    void viewBarcode();
    void productNumberChanged(const QString& text);

    void showDashboard();
    void showBarcodes();
    void showInventory();
    void showProductScan();
    void showAddProduct();
    void showUpdateProduct();

private:
    void setupUI();
    void createSidebar();
    void createMainContent();

    QWidget* createDashboardPage();
    QWidget* createBarcodePage();

    void syncSidebarSelection();
    void resetBarcodePreview();

    IProductRepository* repository;
    ICategoryRepository* categoryRepository = nullptr;

    QWidget* sidebar;
    QWidget* mainContent;
    QHBoxLayout* bodyLayout;
    QStackedWidget* pages;

    QWidget* dashboardPage;
    QWidget* barcodePage;

    QPushButton* dashboardButton;
    QPushButton* barcodesButton;
    QPushButton* inventoryButton;
    QPushButton* productScanButton;

    QLineEdit* productNumberInput;
    QPushButton* viewBarcodeButton;

    QFrame* productsCard;
    QFrame* barcodeCard;
    QLabel* barcodeImage;
    QLabel* barcodeDisplay;
};

#endif