#ifndef IPRODUCTREPOSITORY_H
#define IPRODUCTREPOSITORY_H
#include <string>
#include <vector>
#include "../models/product.h"

class IProductRepository
{
public:
    virtual ~IProductRepository() {}
    virtual bool load() = 0;

    // Finds a product by its barcode. Returns false if not found.
    virtual bool findByBarcode(const std::string& barcode, Product& result) const = 0;

    // Returns all products
    virtual std::vector<Product> getAll() const = 0;

    // Replaces the product (matched by barcode) and saves to the file
    virtual bool update(const Product& product) = 0;

    virtual std::vector<std::string> getErrors() const { return std::vector<std::string>(); }

    virtual bool add(const Product& product, std::string& error) = 0;

    // Old version, the biggest barcode in the file plus 1 (removed in a later commit)
    virtual std::string nextBarcode() const = 0;

    // Next free barcode of a category: categoryId + 4-digit serial (0001 to 9999).
    // Returns "" if the category id is invalid or the category is full.
    virtual std::string nextBarcode(const std::string& categoryId) const = 0;
};

#endif