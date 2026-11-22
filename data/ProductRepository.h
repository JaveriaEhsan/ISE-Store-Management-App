#ifndef PRODUCTREPOSITORY_H
#define PRODUCTREPOSITORY_H
#include<string>
#include <vector>
#include "IProductRepository.h"
using namespace std;

class ProductRepository : public IProductRepository
{
private:
    string filename;
    vector<Product> products;
    vector<string> skippedLines;   // original text of lines that could not be read
    vector<string> errors;         // why each line was skipped

    // Converts one line into a Product. On failure, error says why.
    bool parseLine(string line, Product& p, string& error) const;

    // True if a product with the same barcode or id is already loaded
    bool isDuplicate(const Product& p) const;

    // Writes all products back to the file (skipped lines are kept at the end)
    bool save() const;

    string toLine(const Product& p) const;   // a product as one line of the file
    string nextId() const;

public:
    ProductRepository(const string& filename);

    // Reads the whole file into memory. Returns false if the file cannot be opened.
    bool load()override;

    // How many lines were skipped by load(), and why
    int getSkippedCount() const;
    vector<string> getErrors() const;

    bool findByBarcode(const string& barcode, Product& result) const override;
    vector<Product> getAll() const override;
    bool update(const Product& product) override;
    bool add(const Product& product, string& error) override;
    string nextBarcode() const override;
};

#endif