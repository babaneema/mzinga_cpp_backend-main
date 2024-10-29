#include "activity.hxx"

activity::activity(
        const std::string& unique,
        const std::string& name,
        const int& price,
        const std::string& reg_date 
    ): 
    activity_unique(unique),
    activity_name(name),
    activity_price(price),
    activity_reg_date(reg_date)
{}

const unsigned long activity::id() const {
    return activity_id;
}


const std::string& activity::unique() const {
    return activity_unique;
}


const std::string& activity::name() const {
    return activity_name;
}

const int& activity::price() const {
    return activity_price;
}

const std::string& activity::reg_date() const {
    return activity_reg_date;
}
