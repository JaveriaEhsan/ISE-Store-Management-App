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

    // Finds a product by its id. Returns false if not found.
    virtual bool findById(const std::string& id, Product& result) const = 0;

    // Finds the first product with this acronym (not case sensitive). Returns false if not found.
    virtual bool findByAcronym(const std::string& acronym, Product& result) const = 0;

    // Returns all products
    virtual std::vector<Product> getAll() const = 0;

    // Replaces the product (matched by barcode) and saves to the file.
    // The product is checked with the same rules as add(). The stored id is kept.
    // On failure the old product stays unchanged and error says why.
    virtual bool update(const Product& product, std::string& error) = 0;

    // Same as above when the caller does not need the error text
    bool update(const Product& product)
    {
        std::string error;
        return update(product, error);
    }

    virtual std::vector<std::string> getErrors() const { return std::vector<std::string>(); }

    virtual bool add(const Product& product, std::string& error) = 0;

    // Next free barcode of a category: categoryId + 4-digit serial (0001 to 9999).
    // Returns "" if the category id is invalid or the category is full.
    virtual std::string nextBarcode(const std::string& categoryId) const = 0;
};

#endif