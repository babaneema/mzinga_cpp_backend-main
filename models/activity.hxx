#pragma once
#include <string>
#include <odb/core.hxx>
#include <odb/query.hxx>

#pragma db object 
class activity{

private:
    activity(){}
    friend class odb::access;
    #pragma db id auto  
    unsigned long activity_id;


    std::string activity_unique;
    std::string activity_name;
    int activity_price;
    std::string activity_reg_date;

public:
    activity(
        const std::string& unique,
        const std::string& name,
        const int& price,
        const std::string& reg_date 
    );
    const unsigned long id() const;
    const std::string& unique () const;
    const std::string& name () const;
    const int& price () const;
    const std::string& reg_date() const;
};