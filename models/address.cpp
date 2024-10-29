#include "address.hxx"

address::address(
        const std::string& unique,
        const std::string& name,
        const std::string& reg_date 
    ): 
    address_unique(unique),
    address_name(name),
    address_reg_date(reg_date)
{}

const unsigned long address::id() const {
    return address_id;
}


const std::string& address::unique() const {
    return address_unique;
}


const std::string& address::name() const {
    return address_name;
}


const std::string& address::reg_date() const {
    return address_reg_date;
}
