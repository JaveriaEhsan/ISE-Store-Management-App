#ifndef PRODUCTREPOSITORY_H
#define PRODUCTREPOSITORY_H
#include <string>
#include <vector>
#include "IProductRepository.h"

class ProductRepository : public IProductRepository
{
private:
    std::string filename;
    std::vector<Product> products;
    std::vector<std::string> skippedLines;   // original text of lines that could not be read
    std::vector<std::string> errors;         // why each line was skipped

    // Converts one line into a Product. On failure, error says why.
    bool parseLine(std::string line, Product& p, std::string& error) const;

    // True if a product with the same barcode or id is already loaded
    bool isDuplicate(const Product& p) const;

    // Writes all products back to the file (skipped lines are kept at the end)
    bool save() const;

    std::string toLine(const Product& p) const;   // a product as one line of the file
    std::string nextId() const;

public:
    ProductRepository(const std::string& filename);

    // Reads the whole file into memory. Returns false if the file cannot be opened.
    bool load() override;

    // How many lines were skipped by load(), and why
    int getSkippedCount() const;
    std::vector<std::string> getErrors() const override;

    bool findByBarcode(const std::string& barcode, Product& result) const override;
    std::vector<Product> getAll() const override;
    bool update(const Product& product) override;
    bool add(const Product& product, std::string& error) override;
    std::string nextBarcode() const override;
    std::string nextBarcode(const std::string& categoryId) const override;
};

#endif