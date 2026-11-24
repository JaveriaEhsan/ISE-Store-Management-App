#include <QApplication>
#include <QMessageBox>
#include <QDir>
#include <string>
#include "config.h"
#include "data/ProductRepository.h"
#include "data/OrderRepository.h"
#include "data/OrderDetailRepository.h"
#include "data/BillRepository.h"
#include "data/BillDetailRepository.h"
#include "data/ApprovalRepository.h"
#include "data/SeasonRepository.h"
#include "gui/mainwindow.h"
using namespace std;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // 1. load the products 
    ProductRepository products(PRODUCT_FILE);
    if (!products.load())
    {
        QMessageBox::critical(nullptr, "Product file not found",
            QString("Could not find ") + QString::fromStdString(PRODUCT_FILE) +
            "\n\nThe program was started from:\n" + QDir::currentPath() +
            "\n\nStart it from the project folder.");
        return 1;
    }

    // 2. tell the user about product lines that were skipped
    if (products.getSkippedCount() > 0)
    {
        QString message;
        for (const string& e : products.getErrors())
        {
            message += QString::fromStdString(e) + "\n";
        }
        QMessageBox::warning(nullptr, "Some product lines were skipped", message);
    }

    // 3. the other repositories (they use their own file names for now)
    OrderRepository orders;
    OrderDetailRepository orderDetails;
    BillRepository bills;
    BillDetailRepository billDetails;
    SeasonRepository seasons;
    ApprovalRepository approvals;
    seasons.load();

    // 4. give them all to the main window
    MainWindow window(products, orders, orderDetails, bills, billDetails, seasons, approvals);
    window.show();
    return app.exec();
}