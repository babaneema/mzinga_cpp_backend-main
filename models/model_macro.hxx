#pragma once

#define DEFINE_GETTER_SETTER(type, field_name)    \
    const type get_##field_name() const {         \
        return field_name;                        \
    }                                             \
    void set_##field_name(const type& value) {    \
        field_name = value;                       \
    }                                             \

#define DEFINE_NON_CONST_GETTER_SETTER(type, field_name)    \
    type get_##field_name() const {                         \
        return field_name;                                  \
    }                                                       \
    void set_##field_name(const type& value) {              \
        field_name = value;                                 \
    }                                                       