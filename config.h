#ifndef CONFIG_H
#define CONFIG_H

#include <string>

// ---------- data files (run the program from the project folder) ----------
const std::string PRODUCT_FILE      = "Product.txt";
const std::string CATEGORY_FILE     = "PCategory.txt";     
const std::string ORDER_FILE        = "Order.txt";
const std::string ORDER_DETAIL_FILE = "OrderDetail.txt";
const std::string BILL_FILE         = "Bill.txt";
const std::string BILL_DETAIL_FILE  = "BillDetail.txt";  
const std::string SEASON_FILE       = "Seasons.txt";     

// ---------- business rules ----------
const double MAX_SPECIAL_DISCOUNT_PERCENT = 8.0;   // special discount cap
const int PAID_ORDER_EDIT_DAYS = 7;                // paid orders editable within 7 days
const int LOW_STOCK_LIMIT = 5;                     // low stock alert limit

#endif