#include "mainwindow.h"
#include "cashierwindow.h"
#include "managerwindow.h"
#include "models/approval.h"
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QTimer>
#include <QDate>
#include <QTime>
#include <QFrame>
#include <vector>

using namespace std;

// Small helper: a compact "card" with a number and a caption beside it
static QFrame* makeStatCard(QLabel*& valueLabel, const QString& caption, QWidget* parent)
{
    QFrame* card = new QFrame(parent);
    card->setObjectName("statCard");
    card->setFixedSize(170, 60);
    QHBoxLayout* h = new QHBoxLayout(card);
    h->setContentsMargins(12, 6, 12, 6);
    h->setSpacing(10);

    valueLabel = new QLabel("0", card);
    valueLabel->setObjectName("statValue");

    QLabel* text = new QLabel(caption, card);
    text->setObjectName("statCaption");
    text->setWordWrap(true);

    h->addWidget(valueLabel);
    h->addWidget(text, 1);
    return card;
}

// Puts the button text at the top and a small badge label below it
static QLabel* addBadge(QPushButton* button, const QString& title)
{
    button->setText("");              

    QLabel* titleLabel = new QLabel(title, button);
    titleLabel->setObjectName("buttonTitle");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    QLabel* badge = new QLabel("", button);
    badge->setObjectName("buttonBadge");
    badge->setAlignment(Qt::AlignCenter);
    badge->setAttribute(Qt::WA_TransparentForMouseEvents);
    badge->hide();                         

    QVBoxLayout* layout = new QVBoxLayout(button);
    layout->setContentsMargins(0, 8, 0, 8);
    layout->setSpacing(0);
    layout->addStretch(1);
    layout->addWidget(titleLabel);
    layout->addWidget(badge);
    layout->addStretch(1);
    return badge;
}

MainWindow::MainWindow(IProductRepository& products, ICategoryRepository& categories, OrderRepository& orders,
                       OrderDetailRepository& orderDetails, BillRepository& bills,
                       BillDetailRepository& billDetails, ISeasonRepository& seasons,
                       ApprovalRepository& approvals, QWidget* parent)
    : QMainWindow(parent), products(products), categories(categories), orders(orders),
      orderDetails(orderDetails), bills(bills), billDetails(billDetails),
      seasons(seasons), approvals(approvals)
{
    setWindowTitle("Superstore POS");
    resize(900, 600);
    setMinimumSize(760, 520);

    QWidget* center = new QWidget(this);
    center->setObjectName("root");
    QVBoxLayout* root = new QVBoxLayout(center);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ---------- banner ----------
    QFrame* banner = new QFrame(center);
    banner->setObjectName("banner");
    QVBoxLayout* bannerLayout = new QVBoxLayout(banner);
    bannerLayout->setContentsMargins(30, 25, 30, 25);

    QLabel* title = new QLabel("Superstore POS", banner);
    title->setObjectName("title");
    QLabel* subtitle = new QLabel("Inventory and sales management", banner);
    subtitle->setObjectName("subtitle");
    QLabel* clock = new QLabel(banner);
    clock->setObjectName("clock");

    bannerLayout->addWidget(title);
    bannerLayout->addWidget(subtitle);
    bannerLayout->addWidget(clock);
    root->addWidget(banner);

    // live date and time, updated every second
    auto updateClock = [clock]() {
        clock->setText(QDate::currentDate().toString("dddd, d MMMM yyyy") + "   |   " +
                       QTime::currentTime().toString("hh:mm:ss"));
    };
    updateClock();
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, updateClock);
    timer->start(1000);

    QWidget* body = new QWidget(center);
    QVBoxLayout* bodyLayout = new QVBoxLayout(body);
    bodyLayout->setContentsMargins(40, 20, 40, 10);
    bodyLayout->setSpacing(20);

    cashierButton = new QPushButton("Cashier", body);
    managerButton = new QPushButton("Manager", body);
    cashierButton->setObjectName("cashierButton");
    managerButton->setObjectName("managerButton");
    cashierButton->setFixedSize(360, 90);
    managerButton->setFixedSize(360, 90);
    cashierButton->setCursor(Qt::PointingHandCursor);
    managerButton->setCursor(Qt::PointingHandCursor);

    cashierBadge = addBadge(cashierButton, "Cashier");
    managerBadge = addBadge(managerButton, "Manager");

    bodyLayout->addStretch(1);
    bodyLayout->addWidget(cashierButton, 0, Qt::AlignHCenter);
    bodyLayout->addWidget(managerButton, 0, Qt::AlignHCenter);
    bodyLayout->addStretch(1);

    // ---------- small stat boxes at the bottom ----------
    QHBoxLayout* stats = new QHBoxLayout();
    stats->setSpacing(15);
    stats->addStretch(1);
    stats->addWidget(makeStatCard(totalValue, "Products in store", body));
    stats->addWidget(makeStatCard(lowValue, "Low stock (5 or less)", body));
    stats->addStretch(1);
    bodyLayout->addLayout(stats);

    refreshStats();     // fill in the first numbers
    refreshBadges();

    root->addWidget(body, 1);

    // ---------- footer ----------
    QLabel* footer = new QLabel("ISE Term Project  |  FAST", center);
    footer->setObjectName("footer");
    footer->setAlignment(Qt::AlignCenter);
    root->addWidget(footer);

    setCentralWidget(center);

    setStyleSheet(R"(
        #root { background: #f3f5f9; }

        #banner { background: #1f3a5f; }
        #title { color: white; font-size: 34px; font-weight: bold; }
        #subtitle { color: #c9d6e8; font-size: 16px; }
        #clock { color: #9fb4d0; font-size: 13px; margin-top: 6px; }

        #statCard { background: white; border: 1px solid #dde3ee; border-radius: 8px; }
        #statValue { color: #1f3a5f; font-size: 22px; font-weight: bold; }
        #statCaption { color: #6b7686; font-size: 11px; }

        QPushButton {
            font-size: 24px; font-weight: bold; color: white;
            border: none; border-radius: 12px; padding: 15px;
        }
        #cashierButton { background: #2e8b57; }
        #cashierButton:hover { background: #38a169; }
        #cashierButton:pressed { background: #247046; }
        #managerButton { background: #3b6fb6; }
        #managerButton:hover { background: #4a82cf; }
        #managerButton:pressed { background: #2f5a96; }
        #buttonTitle { color: white; font-size: 24px; font-weight: bold; background: transparent; }
        #buttonBadge { color: #fff3c4; font-size: 13px; font-weight: normal; background: transparent; }

        #footer { color: #8a94a3; font-size: 12px; padding: 10px; }
    )");

    connect(cashierButton, &QPushButton::clicked, this, [this]() {
        CashierWindow* w = new CashierWindow(this->products, this->orders,
                                         this->orderDetails, this->bills,
                                         this->billDetails, this->seasons,
                                         this->approvals);
        w->setAttribute(Qt::WA_DeleteOnClose);
        connect(w, &QObject::destroyed, this, [this]() { refreshStats(); refreshBadges(); show(); });
        hide();
        w->show(); 
    });

    connect(managerButton, &QPushButton::clicked, this, [this]() {
        ManagerWindow* w = new ManagerWindow();
        w->setRepository(&this->products);
        w->setCategoryRepository(&this->categories);
        w->setSeasonRepository(&this->seasons);
        w->setApprovalRepositories(&this->approvals, &this->orders, &this->orderDetails);
        w->setBillRepository(&this->bills);
        w->setBillDetailRepository(&this->billDetails);
        w->setAttribute(Qt::WA_DeleteOnClose);
        connect(w, &QObject::destroyed, this, [this]() { refreshStats(); refreshBadges(); show(); });
        hide();
        w->show();
    });
}

// Re-reads the products and updates the two small boxes at the bottom
void MainWindow::refreshStats()
{
    std::vector<Product> all = products.getAll();
    int low = 0;
    for (const Product& p : all)
    {
        if (p.getQuantity() <= 5) low++;
    }
    totalValue->setText(QString::number((int)all.size()));
    lowValue->setText(QString::number(low));
}

void MainWindow::refreshBadges()
{
    int pending = (int)approvals.findByStatus(APPROVAL_PENDING).size();
    managerBadge->setText(QString("%1 pending bill edits").arg(pending));
    managerBadge->setVisible(pending > 0);

    int ready = (int)approvals.findByStatus(APPROVAL_APPROVED).size();
    cashierBadge->setText(QString("%1 approved bill edits").arg(ready));
    cashierBadge->setVisible(ready > 0);
}