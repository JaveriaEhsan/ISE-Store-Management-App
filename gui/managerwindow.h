#ifndef MANAGERWINDOW_H
#define MANAGERWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include "data/BillRepository.h"
#include "data/BillDetailRepository.h"
#include "../data/IProductRepository.h"
#include "../services/ReportService.h"

class QWidget;
class QPushButton;
class QLineEdit;
class QLabel;
class QFrame;
class QHBoxLayout;
class QStackedWidget;
class ISeasonRepository;
class ICategoryRepository;
class ApprovalRepository;
class OrderRepository;
class OrderDetailRepository;
class ReportService;

class ManagerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ManagerWindow(QWidget *parent = nullptr);
    void setRepository(IProductRepository *repo);
    void setSeasonRepository(ISeasonRepository *repo);
    void setCategoryRepository(ICategoryRepository *repo);
    void setApprovalRepositories(ApprovalRepository *approvals, OrderRepository *orders,
                                 OrderDetailRepository *orderDetails);
    void setBillRepository(BillRepository *bills);
    void setBillDetailRepository(BillDetailRepository *billDetails);
    void setReportService(const ReportService *service);

private slots:
    void viewBarcode();
    void productNumberChanged(const QString &text);

    void showDashboard();
    void showBarcodes();
    void showInventory();
    void showProductScan();
    void showAddProduct();
    void showUpdateProduct();
    void showSeasons();
    void showApprovals();
    void showReports();

private:
    void setupUI();
    void createSidebar();
    void createMainContent();

    QWidget *createDashboardPage();
    QWidget *createBarcodePage();

    void syncSidebarSelection();
    void resetBarcodePreview();
    void updatePendingCount();       // shows how many requests wait for the manager

    IProductRepository *repository;
    ISeasonRepository *seasonRepository = nullptr;
    ICategoryRepository *categoryRepository = nullptr;
    ApprovalRepository *approvalRepository = nullptr;
    OrderRepository *orderRepository = nullptr;
    OrderDetailRepository *orderDetailRepository = nullptr;
    const ReportService *reportService = nullptr;
    BillRepository *billRepository = nullptr;
    BillDetailRepository *billDetailRepository = nullptr;

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
    QPushButton *seasonsButton = nullptr;
    QPushButton *approvalsButton = nullptr;
    QPushButton *reportsButton = nullptr;

    QLineEdit *productNumberInput;
    QPushButton *viewBarcodeButton;

    QFrame *productsCard;
    QFrame *barcodeCard;
    QLabel *barcodeImage;
    QLabel *barcodeDisplay;
};

#endif