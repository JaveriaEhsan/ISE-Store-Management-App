#ifndef IPRODUCTREPOSITORY_H
#define IPRODUCTREPOSITORY_H

#include <vector>
#include "../models/product.h"

class IProductRepository
{
public:
    virtual ~IProductRepository() {}
    virtual bool load() = 0;
    // Finds a product by its barcode. Returns false if not found.
    virtual bool findByBarcode(const string& barcode, Product& result) const = 0;

    // Returns all products
    virtual vector<Product> getAll() const = 0;

    // Replaces the product (matched by barcode) and saves to the file
    virtual bool update(const Product& product) = 0;

    virtual vector<string> getErrors() const { return vector<string>(); }

    virtual bool add(const Product& product, string& error) = 0;

    virtual string nextBarcode() const = 0;
};

#endif
