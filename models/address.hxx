#pragma once
#include <string>
#include <odb/core.hxx>
#include <odb/query.hxx>

#pragma db object 
class address{

private:
    address(){}
    friend class odb::access;
    #pragma db id auto  
    unsigned long address_id;


    std::string address_unique;
    std::string address_name;
    std::string address_reg_date;

public:
    address(
        const std::string& unique,
        const std::string& name,
        const std::string& reg_date 
    );
    const unsigned long id() const;
    const std::string& unique () const;
    const std::string& name () const;
    const std::string& reg_date() const;
};