#ifndef PRODUCT_H
#define PRODUCT_H

#include <string>

// One row of Product.txt
class Product
{
private:
    std::string id;
    std::string categoryId;       // 2 digits, equals the first two digits of barcode
    std::string barcode;          // the 6-digit number encoded in the barcode
    std::string acronym;          // max 8 characters
    std::string name;             // spaces are written as _ in the file
    std::string unit;             // "count" or "weight"
    double price = 0;        // unit price in PKR
    double tax = 0;          // tax per unit in PKR
    double discount = 0;     // discount per unit in PKR
    double quantity = 0;     // available quantity
    std::string barcodePath;      // for example barcodes/123456.png

public:
    Product() : price(0), tax(0), discount(0), quantity(0) {}

    Product(const std::string& id, const std::string& barcode, const std::string& acronym,
        const std::string& name, const std::string& unit, double price, double tax,
        double discount, double quantity, const std::string& barcodePath, const std::string& categoryId = "")
        : id(id), categoryId(categoryId), barcode(barcode), acronym(acronym), name(name), unit(unit),
        price(price), tax(tax), discount(discount), quantity(quantity),
        barcodePath(barcodePath) {
    }

    // ----- getters -----
    std::string getId() const { return id; }
    std::string getCategoryId() const { return categoryId; }
    std::string getBarcode() const { return barcode; }
    std::string getAcronym() const { return acronym; }
    std::string getName() const { return name; }
    std::string getUnit() const { return unit; }
    double getPrice() const { return price; }
    double getTax() const { return tax; }
    double getDiscount() const { return discount; }
    double getQuantity() const { return quantity; }
    std::string getBarcodePath() const { return barcodePath; }

    // ----- methods -----
    bool hasStock(double required) const
    {
        return required > 0 && required <= quantity;
    }

    // Called after a bill is paid.
    // Refuses (returns false, quantity unchanged) if amount is not positive
    // or is more than the stock, so the quantity never goes below zero.
    bool reduceStock(double amount)
    {
        if (amount <= 0 || amount > quantity)
        {
            return false;
        }
        quantity -= amount;
        return true;
    }

    // Called when a product is restocked while editing old orders.
    // Refuses (returns false) if amount is not positive.
    bool addStock(double amount)
    {
        if (amount <= 0)
        {
            return false;
        }
        quantity += amount;
        return true;
    }

    // Used by the repository when it reads a line from Product.txt
    bool isValid() const
    {
        if (id.empty() || barcode.size() != 6) return false;
        if (acronym.empty() || acronym.size() > 8) return false;
        if (price < 0 || tax < 0 || discount < 0 || quantity < 0) return false;

        // category id: 2 digits, equal to the first two barcode digits
        if (categoryId.size() != 2) return false;
        if (barcode.compare(0, 2, categoryId) != 0) return false;
        return true;
    }
};

#endif