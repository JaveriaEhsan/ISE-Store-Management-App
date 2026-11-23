#ifndef DISCOUNTRULES_H
#define DISCOUNTRULES_H

#include <string>
#include "../config.h"   

// Names the calculator uses to sort the discounts into the bill columns
const std::string DISCOUNT_SEASONAL = "seasonal";
const std::string DISCOUNT_SPECIAL = "special";

// One kind of discount on the amount (without tax, after product discounts).
// A new discount type is a new class; BillCalculator does not change.
class IDiscountRule
{
public:
    virtual ~IDiscountRule() {}
    virtual std::string name() const = 0;
    virtual double amount(double base) const = 0;
};

class SeasonalDiscount : public IDiscountRule
{
private:
    double percent;
public:
    explicit SeasonalDiscount(double percent) : percent(percent) {}
    std::string name() const override { return DISCOUNT_SEASONAL; }
    double amount(double base) const override { return base * percent / 100; }
};

class SpecialDiscount : public IDiscountRule
{
private:
    double percent;
public:
    explicit SpecialDiscount(double percent) : percent(percent) {}
    std::string name() const override { return DISCOUNT_SPECIAL; }
    double amount(double base) const override { return base * percent / 100; }

    static bool isValidPercent(double p)
    {
        return p >= 0 && p <= MAX_SPECIAL_DISCOUNT_PERCENT;
    }
};

#endif