#pragma once
#include <string>
#include <odb/core.hxx>
#include <odb/query.hxx>
#include <boost/shared_ptr.hpp>
#include "model_macro.hxx"

#pragma db object pointer(boost::shared_ptr)
class Price{
    friend class odb::access;
    #pragma db id auto  
    unsigned long price_id;

    std::string price_unique;

    std::string price_item;
    std::string price_amount;
    std::string price_reg_date;
public:
    Price(){} 
    Price(const std::string & unique,
            const std::string & item,
            const std::string & amount,
            const std::string & reg_date)
    :
    price_unique(unique), price_item(item), 
    price_amount(amount), price_reg_date(reg_date)
    {}

    DEFINE_GETTER_SETTER(long, price_id)
    DEFINE_GETTER_SETTER(std::string, price_unique)
    DEFINE_GETTER_SETTER(std::string, price_item)
    DEFINE_GETTER_SETTER(std::string, price_amount)
    DEFINE_GETTER_SETTER(std::string, price_reg_date)
};