#ifndef PRODUCT_H
#define PRODUCT_H

#include <string>
using namespace std;

// One row of Product.txt
class Product
{
private:
    string id;
    string barcode;          // the 6-digit number encoded in the barcode
    string acronym;          // max 8 characters
    string name;             // spaces are written as _ in the file
    string unit;             // "count" or "weight"
    double price = 0;        // unit price in PKR
    double tax = 0;          // tax per unit in PKR
    double discount = 0;     // discount per unit in PKR
    double quantity = 0;     // available quantity
    string barcodePath;      // for example barcodes/123456.png

public:
    Product() : price(0), tax(0), discount(0), quantity(0) {}

    Product(const string& id, const string& barcode, const string& acronym,
            const string& name, const string& unit, double price, double tax,
            double discount, double quantity, const string& barcodePath)
        : id(id), barcode(barcode), acronym(acronym), name(name), unit(unit),
          price(price), tax(tax), discount(discount), quantity(quantity),
          barcodePath(barcodePath) {}

    // ----- getters -----
    string getId() const { return id; }
    string getBarcode() const { return barcode; }
    string getAcronym() const { return acronym; }
    string getName() const { return name; }
    string getUnit() const { return unit; }
    double getPrice() const { return price; }
    double getTax() const { return tax; }
    double getDiscount() const { return discount; }
    double getQuantity() const { return quantity; }
    string getBarcodePath() const { return barcodePath; }

    // ----- methods -----
    bool hasStock(double required) const
    {
        return required > 0 && required <= quantity;
    }

    // Called after a bill is paid
    void reduceStock(double amount)
    {
        quantity -= amount;
        if (quantity < 0) quantity = 0;
    }

    // Used by the repository when it reads a line from Product.txt
    bool isValid() const
    {
        return !id.empty() && barcode.size() == 6 && !acronym.empty()
               && acronym.size() <= 8 && price >= 0 && tax >= 0
               && discount >= 0 && quantity >= 0;
    }
};

#endif