#include "managerwindow.h"
#include "inventorywindow.h"
#include "productdialog.h"
#include "addproductdialog.h"
#include "scandialog.h"
#include "seasonwindow.h"
#include "managerapprovaldialog.h"
#include "reportswindow.h"
#include "../models/product.h"
#include "../models/approval.h"
#include "../data/ISeasonRepository.h"
#include "../data/ApprovalRepository.h"
#include "../data/OrderRepository.h"
#include "../data/OrderDetailRepository.h"
#include "../data/ICategoryRepository.h"
#include "../services/ReportService.h"

#include "../shared.h"
#include "../barcode/encoder.h"

#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QMessageBox>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QPainter>
#include <QPixmap>
#include <QImage>
#include <QIcon>
#include <QPen>
#include <QPolygonF>
#include <QtMath>

#include <string>
#include <vector>
using namespace std;

// 6 digits, first digit not zero
bool validNumber(const QString &number)
{
    return isvalidInput(number.toStdString());
}

QImage buildImage(const QString &digits)
{
    string bits = Encode(digits.toStdString());
    if (bits.empty())
        return QImage();

    int width = (int(bits.size()) + 2 * Quiet_Zone) * Bar_Width;
    QImage img(width, Image_Height, QImage::Format_RGB32);
    img.fill(Qt::white);

    QPainter p(&img);
    for (size_t i = 0; i < bits.size(); ++i)
    {
        if (bits[i] == '1')
            p.fillRect((Quiet_Zone + int(i)) * Bar_Width, 0,
                       Bar_Width, Image_Height, Qt::black);
    }
    p.end();
    return img;
}

QString barcodeFolder()
{
    return QDir::current().filePath("barcodes");
}

QString barcodePath(const QString &number)
{
    return QDir(barcodeFolder()).filePath(number + ".png");
}


enum IconType { IconDashboard, IconBarcode, IconCube, IconScan, IconBasket,
                IconSeason, IconApproval, IconReport };

QPixmap makeIcon(IconType type, const QColor &color, int size)
{
    const qreal dpr = 2.0;
    QPixmap pm(int(size * dpr), int(size * dpr));
    pm.fill(Qt::transparent);
    pm.setDevicePixelRatio(dpr);

    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);

    QPen pen(color, size / 13.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);

    const qreal s = size;
    const qreal m = s * 0.12;

    switch (type)
    {
    case IconDashboard:
    {
        qreal g = s * 0.10;
        qreal w = (s - 2 * m - g) / 2;
        qreal r = s * 0.08;
        p.drawRoundedRect(QRectF(m, m, w, w), r, r);
        p.drawRoundedRect(QRectF(m + w + g, m, w, w), r, r);
        p.drawRoundedRect(QRectF(m, m + w + g, w, w), r, r);
        p.drawRoundedRect(QRectF(m + w + g, m + w + g, w, w), r, r);
        break;
    }
    case IconBarcode:
    {
        for (int i = 0; i <= 6; ++i)
        {
            qreal x = m + i * (s - 2 * m) / 6.0;
            QPen bar(color, (i % 2 == 0) ? s / 8.0 : s / 16.0,
                     Qt::SolidLine, Qt::FlatCap);
            p.setPen(bar);
            p.drawLine(QPointF(x, m), QPointF(x, s - m));
        }
        break;
    }
    case IconCube:
    {
        QPointF c(s / 2, s / 2);
        qreal r = s * 0.40;
        QPolygonF hex;
        QPointF v[6];
        for (int i = 0; i < 6; ++i)
        {
            qreal a = qDegreesToRadians(-90.0 + 60.0 * i);
            v[i] = QPointF(c.x() + r * qCos(a), c.y() + r * qSin(a));
            hex << v[i];
        }
        p.drawPolygon(hex);
        p.drawLine(c, v[1]);
        p.drawLine(c, v[3]);
        p.drawLine(c, v[5]);
        break;
    }
    case IconScan:
    {
        qreal l = s * 0.28;
        qreal a = m, b = s - m;
        p.drawLine(QPointF(a, a + l), QPointF(a, a));
        p.drawLine(QPointF(a, a), QPointF(a + l, a));
        p.drawLine(QPointF(b - l, a), QPointF(b, a));
        p.drawLine(QPointF(b, a), QPointF(b, a + l));
        p.drawLine(QPointF(a, b - l), QPointF(a, b));
        p.drawLine(QPointF(a, b), QPointF(a + l, b));
        p.drawLine(QPointF(b - l, b), QPointF(b, b));
        p.drawLine(QPointF(b, b), QPointF(b, b - l));
        p.drawLine(QPointF(a + s * 0.08, s / 2), QPointF(b - s * 0.08, s / 2));
        break;
    }
    case IconBasket:
    {
        QPolygonF body;
        body << QPointF(m, s * 0.42) << QPointF(s - m, s * 0.42)
             << QPointF(s * 0.82, s - m) << QPointF(s * 0.18, s - m);
        p.drawPolygon(body);
        p.drawLine(QPointF(s * 0.28, s * 0.42), QPointF(s * 0.42, s * 0.16));
        p.drawLine(QPointF(s * 0.72, s * 0.42), QPointF(s * 0.58, s * 0.16));
        p.drawLine(QPointF(s * 0.40, s * 0.55), QPointF(s * 0.40, s * 0.78));
        p.drawLine(QPointF(s * 0.50, s * 0.55), QPointF(s * 0.50, s * 0.78));
        p.drawLine(QPointF(s * 0.60, s * 0.55), QPointF(s * 0.60, s * 0.78));
        break;
    }
    case IconSeason:
    {
        // a percent sign: a slanted line with a small circle at each end
        qreal r = s * 0.13;
        p.drawLine(QPointF(s * 0.78, s * 0.20), QPointF(s * 0.22, s * 0.80));
        p.drawEllipse(QPointF(s * 0.28, s * 0.28), r, r);
        p.drawEllipse(QPointF(s * 0.72, s * 0.72), r, r);
        break;
    }
    case IconApproval:
    {
        // a check mark inside a circle
        p.drawEllipse(QPointF(s / 2, s / 2), s * 0.38, s * 0.38);
        p.drawLine(QPointF(s * 0.32, s * 0.52), QPointF(s * 0.45, s * 0.65));
        p.drawLine(QPointF(s * 0.45, s * 0.65), QPointF(s * 0.70, s * 0.36));
        break;
    }
    case IconReport:
    {
        // three bars standing on a line
        qreal base = s - m;
        qreal bw = s * 0.16;
        p.drawLine(QPointF(m, base), QPointF(s - m, base));
        p.drawRect(QRectF(s * 0.20, s * 0.52, bw, base - s * 0.52));
        p.drawRect(QRectF(s * 0.42, s * 0.28, bw, base - s * 0.28));
        p.drawRect(QRectF(s * 0.64, s * 0.40, bw, base - s * 0.40));
        break;
    }
    }

    p.end();
    return pm;
}

QFrame *makeCard(IconType type, const QColor &iconColor,
                 const QString &title, const QString &subtitle)
{
    QFrame *card = new QFrame;
    card->setObjectName("dashCard");
    card->setMinimumHeight(150);
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    card->setStyleSheet(
        "QFrame#dashCard {"
        "background-color: white;"
        "border: 1px solid #E2E6EF;"
        "border-radius: 14px;"
        "}"
        "QLabel { background: transparent; border: none; }"
    );

    QVBoxLayout *lay = new QVBoxLayout(card);
    lay->setContentsMargins(24, 18, 18, 18);
    lay->setSpacing(4);

    QLabel *icon = new QLabel;
    icon->setPixmap(makeIcon(type, iconColor, 34));
    icon->setFixedSize(34, 34);

    QLabel *t = new QLabel(title);
    t->setStyleSheet("font-size: 26px; font-weight: bold; color: #172342;");

    QLabel *s = new QLabel(subtitle);
    s->setStyleSheet("font-size: 14px; color: #7A8294;");

    lay->addWidget(icon);
    lay->addStretch();
    lay->addWidget(t);
    lay->addWidget(s);

    return card;
}

QString primaryButtonStyle()
{
    return "QPushButton {"
           "background-color: #1761D1; color: white;"
           "font-size: 18px; font-weight: bold; border-radius: 9px;"
           "}"
           "QPushButton:hover { background-color: #104AA9; }"
           "QPushButton:disabled { background-color: #A9B8D6; }";
}

QString secondaryButtonStyle()
{
    return "QPushButton {"
           "background-color: #E8ECF4; color: #34415C;"
           "font-size: 17px; font-weight: bold; border-radius: 9px;"
           "}"
           "QPushButton:hover { background-color: #D9E0EF; }";
}

QString inputStyle()
{
    return "QLineEdit {"
           "background-color: #F4F6FA;"
           "border: 1px solid #D9DEE8;"
           "border-radius: 8px;"
           "padding-left: 15px;"
           "font-size: 17px;"
           "color: #172342;"
           "}"
           "QLineEdit:focus { border: 2px solid #1761D1; }";
}

QString panelStyle()
{
    return "QFrame#panel {"
           "background-color: white;"
           "border: 1px solid #E2E6EF;"
           "border-radius: 14px;"
           "}"
           "QLabel { background: transparent; border: none; }";
}

QString pageHeadingStyle()
{
    return "font-size: 32px; color: #101A35; font-weight: bold;";
}

ManagerWindow::ManagerWindow(QWidget *parent)
    : QMainWindow(parent)
{
    repository = nullptr;
    sidebar = nullptr;
    mainContent = nullptr;
    bodyLayout = nullptr;
    pages = nullptr;

    dashboardPage = nullptr;
    barcodePage = nullptr;

    dashboardButton = nullptr;
    barcodesButton = nullptr;
    inventoryButton = nullptr;
    productScanButton = nullptr;

    productNumberInput = nullptr;
    viewBarcodeButton = nullptr;

    productsCard = nullptr;
    barcodeCard = nullptr;
    barcodeImage = nullptr;
    barcodeDisplay = nullptr;

    setupUI();
}

void ManagerWindow::setRepository(IProductRepository *repo)
{
    repository = repo;
}

void ManagerWindow::setSeasonRepository(ISeasonRepository *repo)
{
    seasonRepository = repo;
}

void ManagerWindow::setApprovalRepositories(ApprovalRepository *approvals, OrderRepository *orders,
                                            OrderDetailRepository *orderDetails)
{
    approvalRepository = approvals;
    orderRepository = orders;
    orderDetailRepository = orderDetails;

    updatePendingCount();       // show how many requests are waiting right now
}

void ManagerWindow::setCategoryRepository(ICategoryRepository* repo)
{
    categoryRepository = repo;
}

void ManagerWindow::setReportService(ReportService *service)
{
    reportService = service;
}

// "Approvals (2)" when two requests wait for the manager, just "Approvals" when none
void ManagerWindow::updatePendingCount()
{
    if (approvalsButton == nullptr)
        return;

    QString text = "  Approvals";

    if (approvalRepository != nullptr)
    {
        int waiting = (int)approvalRepository->findByStatus(APPROVAL_PENDING).size();
        if (waiting > 0)
            text += " (" + QString::number(waiting) + ")";
    }

    approvalsButton->setText(text);
}

void ManagerWindow::setupUI()
{
    setWindowTitle("SmartPOS - Manager Dashboard");
    resize(1250, 780);
    setMinimumSize(950, 650);

    QWidget *central = new QWidget(this);
    central->setStyleSheet("background-color: #F4F6FA;");
    setCentralWidget(central);

    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QWidget *header = new QWidget;
    header->setFixedHeight(120);
    header->setStyleSheet("background-color: #07143E;");

    QVBoxLayout *headerLayout = new QVBoxLayout(header);
    headerLayout->setContentsMargins(28, 16, 28, 14);
    headerLayout->setSpacing(6);

    QHBoxLayout *brandLayout = new QHBoxLayout;
    brandLayout->setSpacing(12);

    QLabel *logoIcon = new QLabel;
    logoIcon->setPixmap(makeIcon(IconBasket, Qt::white, 36));
    logoIcon->setFixedSize(36, 36);

    QLabel *logo = new QLabel("SmartPOS");
    logo->setStyleSheet("color: white; font-size: 30px; font-weight: bold;");

    QLabel *managerBadge = new QLabel("Manager");
    managerBadge->setAlignment(Qt::AlignCenter);
    managerBadge->setFixedSize(100, 34);
    managerBadge->setStyleSheet(
        "background-color: #15952C;"
        "color: white;"
        "font-size: 15px;"
        "font-weight: bold;"
        "border-radius: 17px;"
    );

    // Back button: closes this window, the main window shows again
    QPushButton *backButton = new QPushButton("< Back", this);
    backButton->setFixedSize(90, 34);
    backButton->setCursor(Qt::PointingHandCursor);
    backButton->setStyleSheet(
        "QPushButton {"
        "background-color: transparent;"
        "color: white;"
        "font-size: 15px;"
        "font-weight: bold;"
        "border: 2px solid white;"
        "border-radius: 17px;"
        "}"
        "QPushButton:hover { background-color: #1B3485; }"
    );

    brandLayout->addWidget(logoIcon);
    brandLayout->addWidget(logo);
    brandLayout->addStretch();
    brandLayout->addWidget(backButton);
    brandLayout->addSpacing(10);
    brandLayout->addWidget(managerBadge);

    connect(backButton, &QPushButton::clicked, this, &QWidget::close);

    QLabel *subtitle = new QLabel("Store Management Dashboard");
    subtitle->setStyleSheet("color: white; font-size: 17px;");

    headerLayout->addLayout(brandLayout);
    headerLayout->addWidget(subtitle);

    mainLayout->addWidget(header);

    bodyLayout = new QHBoxLayout;
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(0);

    mainLayout->addLayout(bodyLayout, 1);

    createSidebar();
    createMainContent();
}

void ManagerWindow::createSidebar()
{
    sidebar = new QWidget;
    sidebar->setFixedWidth(300);
    sidebar->setStyleSheet("background-color: #0A1F6B;");

    QVBoxLayout *sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(18, 28, 18, 20);
    sideLayout->setSpacing(14);

    dashboardButton = new QPushButton("  Dashboard");
    barcodesButton = new QPushButton("  View Barcode");
    inventoryButton = new QPushButton("  Inventory");
    productScanButton = new QPushButton("  Product Scan");
    seasonsButton = new QPushButton("  Seasons");
    approvalsButton = new QPushButton("  Approvals");
    reportsButton = new QPushButton("  Reports");

    dashboardButton->setIcon(QIcon(makeIcon(IconDashboard, Qt::white, 26)));
    barcodesButton->setIcon(QIcon(makeIcon(IconBarcode, Qt::white, 26)));
    inventoryButton->setIcon(QIcon(makeIcon(IconCube, Qt::white, 26)));
    productScanButton->setIcon(QIcon(makeIcon(IconScan, Qt::white, 26)));
    seasonsButton->setIcon(QIcon(makeIcon(IconSeason, Qt::white, 26)));
    approvalsButton->setIcon(QIcon(makeIcon(IconApproval, Qt::white, 26)));
    reportsButton->setIcon(QIcon(makeIcon(IconReport, Qt::white, 26)));

    QPushButton *buttons[] = {
        dashboardButton, barcodesButton, inventoryButton, productScanButton,
        seasonsButton, approvalsButton, reportsButton
    };

    for (QPushButton *button : buttons)
    {
        button->setIconSize(QSize(26, 26));
        button->setMinimumHeight(52);
        button->setCursor(Qt::PointingHandCursor);
        button->setCheckable(true);
        button->setAutoExclusive(true);
        button->setStyleSheet(
            "QPushButton {"
            "color: white;"
            "background-color: transparent;"
            "border: none;"
            "text-align: left;"
            "padding-left: 12px;"
            "font-size: 18px;"
            "font-weight: bold;"
            "border-radius: 8px;"
            "}"
            "QPushButton:hover { background-color: #1B3485; }"
            "QPushButton:checked { background-color: #1761D1; }"
        );
        sideLayout->addWidget(button);
    }

    dashboardButton->setChecked(true);

    sideLayout->addStretch();
    bodyLayout->addWidget(sidebar);

    connect(dashboardButton, &QPushButton::clicked,
            this, &ManagerWindow::showDashboard);
    connect(barcodesButton, &QPushButton::clicked,
            this, &ManagerWindow::showBarcodes);
    connect(inventoryButton, &QPushButton::clicked,
            this, &ManagerWindow::showInventory);
    connect(productScanButton, &QPushButton::clicked,
            this, &ManagerWindow::showProductScan);
    connect(seasonsButton, &QPushButton::clicked,
            this, &ManagerWindow::showSeasons);
    connect(approvalsButton, &QPushButton::clicked,
            this, &ManagerWindow::showApprovals);
    connect(reportsButton, &QPushButton::clicked,
            this, &ManagerWindow::showReports);
}

void ManagerWindow::createMainContent()
{
    mainContent = new QWidget;
    mainContent->setStyleSheet("background-color: #F4F6FA;");

    QVBoxLayout *contentLayout = new QVBoxLayout(mainContent);
    contentLayout->setContentsMargins(34, 28, 34, 25);

    pages = new QStackedWidget;

    dashboardPage = createDashboardPage();
    barcodePage = createBarcodePage();

    pages->addWidget(dashboardPage);
    pages->addWidget(barcodePage);

    contentLayout->addWidget(pages);
    bodyLayout->addWidget(mainContent, 1);

    pages->setCurrentWidget(dashboardPage);
}

QWidget *ManagerWindow::createDashboardPage()
{
    QWidget *page = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(18);

    QLabel *welcome = new QLabel("Welcome, Manager");
    welcome->setStyleSheet(pageHeadingStyle());

    QLabel *description = new QLabel("Manage your store operations");
    description->setStyleSheet("font-size: 17px; color: #606878;");

    layout->addWidget(welcome);
    layout->addWidget(description);

    QHBoxLayout *cardsLayout = new QHBoxLayout;
    cardsLayout->setSpacing(22);

    productsCard = makeCard(IconCube, QColor("#1761D1"),
                            "Products", "Inventory");
    barcodeCard = makeCard(IconBarcode, QColor("#15952C"),
                           "Barcode", "Viewer");

    cardsLayout->addWidget(productsCard);
    cardsLayout->addWidget(barcodeCard);
    layout->addLayout(cardsLayout);

    QFrame *quick = new QFrame;
    quick->setObjectName("panel");
    quick->setStyleSheet(panelStyle());

    QVBoxLayout *quickLayout = new QVBoxLayout(quick);
    quickLayout->setContentsMargins(24, 22, 24, 22);
    quickLayout->setSpacing(12);

    QLabel *quickHeading = new QLabel("Quick Actions");
    quickHeading->setStyleSheet(
        "font-size: 24px; font-weight: bold; color: #172342;");

    QPushButton *addProduct = new QPushButton("Add New Product");
    addProduct->setStyleSheet(primaryButtonStyle());

    QPushButton *updateProduct = new QPushButton("Update Price / Quantity");
    updateProduct->setStyleSheet(secondaryButtonStyle());

    QPushButton *openBarcodes = new QPushButton("View Barcode");
    openBarcodes->setStyleSheet(secondaryButtonStyle());

    QPushButton *openInventory = new QPushButton("Inventory");
    openInventory->setStyleSheet(secondaryButtonStyle());

    QPushButton *openScan = new QPushButton("View product by scanning");
    openScan->setStyleSheet(secondaryButtonStyle());

    QPushButton *actionButtons[] = {
        addProduct, updateProduct, openBarcodes, openInventory, openScan
    };

    quickLayout->addWidget(quickHeading);
    for (QPushButton *b : actionButtons)
    {
        b->setMinimumHeight(48);
        b->setCursor(Qt::PointingHandCursor);
        quickLayout->addWidget(b);
    }

    layout->addWidget(quick);
    layout->addStretch();

    connect(addProduct, &QPushButton::clicked,
            this, &ManagerWindow::showAddProduct);
    connect(updateProduct, &QPushButton::clicked,
            this, &ManagerWindow::showUpdateProduct);
    connect(openBarcodes, &QPushButton::clicked,
            this, &ManagerWindow::showBarcodes);
    connect(openInventory, &QPushButton::clicked,
            this, &ManagerWindow::showInventory);
    connect(openScan, &QPushButton::clicked,
            this, &ManagerWindow::showProductScan);

    return page;
}

QWidget *ManagerWindow::createBarcodePage()
{
    QWidget *page = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(18);

    QLabel *heading = new QLabel("View Barcode");
    heading->setStyleSheet(pageHeadingStyle());

    QLabel *description = new QLabel(
        "Enter the 6-digit barcode of a product to see its barcode image");
    description->setStyleSheet("font-size: 17px; color: #606878;");

    layout->addWidget(heading);
    layout->addWidget(description);

    QFrame *panel = new QFrame;
    panel->setObjectName("panel");
    panel->setStyleSheet(panelStyle());

    QVBoxLayout *panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(24, 22, 24, 22);
    panelLayout->setSpacing(14);

    productNumberInput = new QLineEdit;
    productNumberInput->setPlaceholderText("Enter 6-digit barcode");
    productNumberInput->setMaxLength(6);
    productNumberInput->setMinimumHeight(48);
    productNumberInput->setValidator(
        new QRegularExpressionValidator(
            QRegularExpression("[0-9]{0,6}"), productNumberInput));
    productNumberInput->setStyleSheet(inputStyle());

    viewBarcodeButton = new QPushButton("View Barcode");
    viewBarcodeButton->setMinimumHeight(50);
    viewBarcodeButton->setCursor(Qt::PointingHandCursor);
    viewBarcodeButton->setStyleSheet(primaryButtonStyle());

    QFrame *preview = new QFrame;
    preview->setStyleSheet(
        "QFrame {"
        "background-color: #F8FAFD;"
        "border: 1px dashed #AAB8D4;"
        "border-radius: 10px;"
        "}"
        "QLabel { background: transparent; border: none; }"
    );
    preview->setMinimumHeight(200);

    QVBoxLayout *previewLayout = new QVBoxLayout(preview);
    previewLayout->setContentsMargins(10, 10, 10, 10);

    barcodeImage = new QLabel;
    barcodeImage->setAlignment(Qt::AlignCenter);
    barcodeImage->setPixmap(makeIcon(IconBarcode, QColor("#AAB8D4"), 60));

    barcodeDisplay = new QLabel("Barcode image will appear here");
    barcodeDisplay->setAlignment(Qt::AlignCenter);
    barcodeDisplay->setStyleSheet("font-size: 15px; color: #606878;");

    previewLayout->addWidget(barcodeImage, 1);
    previewLayout->addWidget(barcodeDisplay);

    panelLayout->addWidget(productNumberInput);
    panelLayout->addWidget(viewBarcodeButton);
    panelLayout->addWidget(preview);

    layout->addWidget(panel);
    layout->addStretch();

    connect(viewBarcodeButton, &QPushButton::clicked,
            this, &ManagerWindow::viewBarcode);
    connect(productNumberInput, &QLineEdit::returnPressed,
            this, &ManagerWindow::viewBarcode);
    connect(productNumberInput, &QLineEdit::textChanged,
            this, &ManagerWindow::productNumberChanged);

    return page;
}

void ManagerWindow::resetBarcodePreview()
{
    barcodeImage->setPixmap(makeIcon(IconBarcode, QColor("#AAB8D4"), 60));
    barcodeDisplay->setText("Barcode image will appear here");
    barcodeDisplay->setStyleSheet("font-size: 15px; color: #606878;");
}

// Error as soon as 6 digits are typed with a leading zero
void ManagerWindow::productNumberChanged(const QString &text)
{
    if (text.length() == 6 && !validNumber(text))
    {
        productNumberInput->clear();
        resetBarcodePreview();

        QMessageBox::warning(
            this, "Invalid Input",
            "Invalid barcode.\nThe first digit cannot be zero.");
    }
}

// View Barcode: shows the image if the product exists
void ManagerWindow::viewBarcode()
{
    QString number = productNumberInput->text();

    if (!validNumber(number))
    {
        productNumberInput->clear();
        resetBarcodePreview();

        QMessageBox::warning(
            this, "Invalid Input",
            "Please enter exactly 6 digits.\nThe first digit cannot be zero.");
        return;
    }

    if (repository == nullptr)
    {
        QMessageBox::information(
            this, "View Barcode", "Inventory is not connected yet.");
        return;
    }

    repository->load();
    Product product;
    if (!repository->findByBarcode(number.toStdString(), product))
    {
        resetBarcodePreview();
        barcodeDisplay->setText("Product does not exist: " + number);
        barcodeDisplay->setStyleSheet("font-size: 16px; color: #C62828;");

        QMessageBox::warning(
            this, "View Barcode",
            "Product does not exist.\nNo product has the barcode " + number + ".");
        return;
    }

    // use the saved image; if the file is missing, make it again
    QString path = barcodePath(number);
    QImage img;
    if (QFile::exists(path))
        img.load(path);

    if (img.isNull())
    {
        QDir().mkpath(barcodeFolder());
        img = buildImage(number);
        if (img.isNull() || !img.save(path, "PNG"))
        {
            QMessageBox::warning(
                this, "Error", "Could not create the barcode image:\n" + path);
            return;
        }
    }

    barcodeImage->setPixmap(QPixmap::fromImage(img).scaled(
        img.width() * 3, img.height() * 3,
        Qt::KeepAspectRatio, Qt::FastTransformation));

    barcodeDisplay->setText(
        number + " - " + QString::fromStdString(product.getName()));
    barcodeDisplay->setStyleSheet("font-size: 16px; color: #15952C;");
}

void ManagerWindow::syncSidebarSelection()
{
    if (pages->currentWidget() == barcodePage)
        barcodesButton->setChecked(true);
    else
        dashboardButton->setChecked(true);
}

void ManagerWindow::showDashboard()
{
    dashboardButton->setChecked(true);
    pages->setCurrentWidget(dashboardPage);
}

void ManagerWindow::showBarcodes()
{
    barcodesButton->setChecked(true);
    pages->setCurrentWidget(barcodePage);
    productNumberInput->setFocus();
}

void ManagerWindow::showInventory()
{
    if (repository == nullptr || categoryRepository == nullptr)
    {
        QMessageBox::information(
            this, "Inventory", "Inventory is not connected yet.");
    }
    else
    {
        InventoryWindow dialog(repository, categoryRepository, this);
        dialog.exec();
    }

    syncSidebarSelection();
}

// Add New Product: form -> Product.txt line -> barcode image
void ManagerWindow::showAddProduct()
{
    if (repository == nullptr || categoryRepository == nullptr)
    {
        QMessageBox::information(
            this, "Add New Product", "Inventory is not connected yet.");
        syncSidebarSelection();
        return;
    }

    AddProductDialog dialog(*repository, *categoryRepository, this);

    if (dialog.exec() == QDialog::Accepted)
    {
        Product p = dialog.createdProduct();
        QString number = QString::fromStdString(p.getBarcode());

        QDir().mkpath(barcodeFolder());
        QImage img = buildImage(number);

        if (!img.isNull() && img.save(barcodePath(number), "PNG"))
        {
            barcodeImage->setPixmap(QPixmap::fromImage(img).scaled(
                img.width() * 3, img.height() * 3,
                Qt::KeepAspectRatio, Qt::FastTransformation));
            barcodeDisplay->setText(number + " - product added, image saved");

            QMessageBox::information(
                this, "Add New Product",
                "Product saved.\nBarcode: " + number +
                "\nImage: barcodes/" + number + ".png");
        }
        else
        {
            QMessageBox::warning(
                this, "Add New Product",
                "The product was saved, but the barcode image could not be created.\n"
                "Open View Barcode to try again.");
        }
    }

    syncSidebarSelection();
}

// Update Price / Quantity: new price, and stock is ADDED to the old quantity
void ManagerWindow::showUpdateProduct()
{
    if (repository == nullptr)
    {
        QMessageBox::information(
            this, "Update Product", "Inventory is not connected yet.");
        syncSidebarSelection();
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Update Price / Quantity");
    dialog.setMinimumWidth(430);

    QLineEdit *barcodeInput = new QLineEdit;
    barcodeInput->setPlaceholderText("6-digit barcode");
    barcodeInput->setMaxLength(6);
    barcodeInput->setValidator(new QRegularExpressionValidator(
        QRegularExpression("[0-9]{0,6}"), barcodeInput));

    QLabel *info = new QLabel("Enter the 6-digit barcode of the product.");
    info->setWordWrap(true);

    QDoubleSpinBox *priceBox = new QDoubleSpinBox;
    priceBox->setRange(0.0, 10000000.0);
    priceBox->setDecimals(2);
    priceBox->setEnabled(false);

    QDoubleSpinBox *addBox = new QDoubleSpinBox;
    addBox->setRange(0.0, 10000000.0);
    addBox->setDecimals(0);
    addBox->setEnabled(false);

    QPushButton *saveButton = new QPushButton("Save");
    saveButton->setEnabled(false);
    QPushButton *cancelButton = new QPushButton("Cancel");

    QFormLayout *form = new QFormLayout;
    form->addRow("Barcode:", barcodeInput);
    form->addRow("", info);
    form->addRow("New unit price (PKR):", priceBox);
    form->addRow("Stock to add:", addBox);

    QHBoxLayout *buttons = new QHBoxLayout;
    buttons->addWidget(saveButton);
    buttons->addWidget(cancelButton);

    QVBoxLayout *lay = new QVBoxLayout(&dialog);
    lay->addLayout(form);
    lay->addLayout(buttons);

    Product found;
    bool hasProduct = false;

    connect(barcodeInput, &QLineEdit::textChanged, &dialog,
        [&](const QString &text)
        {
            hasProduct = false;
            priceBox->setEnabled(false);
            addBox->setEnabled(false);
            saveButton->setEnabled(false);
            info->setStyleSheet("");

            if (text.length() != 6)
            {
                info->setText("Enter the 6-digit barcode of the product.");
                return;
            }
            if (!validNumber(text))
            {
                info->setText("Invalid barcode. The first digit cannot be 0.");
                info->setStyleSheet("color: #C62828;");
                return;
            }

            repository->load();
            if (!repository->findByBarcode(text.toStdString(), found))
            {
                info->setText("Product does not exist.");
                info->setStyleSheet("color: #C62828;");
                return;
            }

            hasProduct = true;
            info->setText(
                QString::fromStdString(found.getName()) +
                "\nCurrent price: " + QString::number(found.getPrice(), 'f', 2) +
                " PKR\nCurrent quantity: " +
                QString::number(found.getQuantity(), 'g', 10));
            info->setStyleSheet("color: #15952C;");

            priceBox->setValue(found.getPrice());
            addBox->setDecimals(found.getUnit() == "count" ? 0 : 3);
            addBox->setValue(0);
            priceBox->setEnabled(true);
            addBox->setEnabled(true);
            saveButton->setEnabled(true);
        });

    connect(saveButton, &QPushButton::clicked, &dialog, [&]()
        {
            if (!hasProduct)
                return;

            Product updated(found.getId(), found.getBarcode(),
                            found.getAcronym(), found.getName(),
                            found.getUnit(), priceBox->value(),
                            found.getTax(), found.getDiscount(),
                            found.getQuantity() + addBox->value(),
                            found.getBarcodePath());

            if (!repository->update(updated))
            {
                QMessageBox::warning(
                    &dialog, "Update Product",
                    "The product could not be updated.");
                return;
            }

            QMessageBox::information(
                &dialog, "Update Product",
                "Saved.\nNew price: " + QString::number(updated.getPrice(), 'f', 2) +
                " PKR\nNew quantity: " +
                QString::number(updated.getQuantity(), 'g', 10));
            dialog.accept();
        });

    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    dialog.exec();
    syncSidebarSelection();
}

// Scan dialog (camera or typed) -> product dialog
void ManagerWindow::showProductScan()
{
    if (repository == nullptr)
    {
        QMessageBox::information(
            this, "Product Scan", "Inventory is not connected yet.");
        syncSidebarSelection();
        return;
    }

    ScanDialog scan(this);

    if (scan.exec() == QDialog::Accepted)
    {
        QString number = scan.barcode();

        repository->load();
        Product product;
        if (repository->findByBarcode(number.toStdString(), product))
        {
            ProductDialog dialog(product, this);
            dialog.exec();
        }
        else
        {
            QMessageBox::information(
                this, "Product not found",
                "Product does not exist.\nNo product with barcode " + number + ".");
        }
    }

    syncSidebarSelection();
}

// Seasons: add a season, switch it on or off
void ManagerWindow::showSeasons()
{
    if (seasonRepository == nullptr)
    {
        QMessageBox::information(
            this, "Seasons", "Seasons are not connected yet.");
        syncSidebarSelection();
        return;
    }

    SeasonWindow dialog(*seasonRepository, this);
    dialog.exec();

    syncSidebarSelection();
}

// Approvals: the cashier asked to edit a paid order, the manager approves or rejects it
void ManagerWindow::showApprovals()
{
    if (approvalRepository == nullptr || orderRepository == nullptr
        || orderDetailRepository == nullptr)
    {
        QMessageBox::information(
            this, "Approvals", "Approvals are not connected yet.");
        syncSidebarSelection();
        return;
    }

    ManagerApprovalDialog dialog(*approvalRepository, *orderRepository,
                                 *orderDetailRepository, this);
    dialog.exec();

    updatePendingCount();
    syncSidebarSelection();
}

// Reports: one report at a time (the window only shows what ReportService gives)
void ManagerWindow::showReports()
{
    if (reportService == nullptr)
    {
        QMessageBox::information(
            this, "Reports", "Reports are not connected yet.");
        syncSidebarSelection();
        return;
    }

    ReportsWindow dialog(*reportService, this);
    dialog.exec();

    syncSidebarSelection();
}