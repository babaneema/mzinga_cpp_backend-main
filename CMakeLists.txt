cmake_minimum_required(VERSION 3.16)
project(MyProject)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(ODB_FLAGS --std c++14)


# add_subdirectory(jwt-cpp-master)
# include_directories(${CMAKE_SOURCE_DIR}/jwt-cpp-master/include) 

set(SOURCES
    main.cpp
   
    models/branch-odb.cxx
    models/address.cpp
    models/address-odb.cxx
    models/employee-odb.cxx
    models/checkin.cpp 
    models/checkin-odb.cxx
    models/activity.cpp 
    models/activity-odb.cxx
    models/bill-odb.cxx
    models/customer-odb.cxx
    models/unit-odb.cxx
    models/meter-odb.cxx
    models/payment-odb.cxx
    models/companies-odb.cxx 
    models/users-odb.cxx

)

set(HEADERS
    models/model_macro.hxx
    models/branch.hxx
    models/branch-odb.hxx
    models/address.hxx
    models/address-odb.hxx
    models/employee.hxx
    models/employee.odb.hxx
    models/checkin.hxx
    models/checkin-odb.hxx
    models/activity.hxx
    models/activity-odb.hxx
    models/bill.hxx
    models/bill-odb.hxx
    models/customer.hxx
    models/customer-odb.hxx
    models/unit.hxx
    models/unit-odb.hxx
    models/meter.hxx
    models/meter-odb.hxx
    models/payment.hxx
    models/payment-odb.hxx
    models/companies.hxx
    models/companies-odb.hxx 
    models/users.hxx
    models/users-odb.hxx 
    
    core/database.hxx
    core/server.hxx
    core/safe_json.hxx
    core/helpers.hxx
    core/sms.hxx
    core/session.hxx
    core/session_macro.hxx
    
    controllers/addressController.hxx
    controllers/branchController.hxx
    controllers/employeeController.hxx
    controllers/checkinController.hxx
    controller/activityController.hxx 
    controller/billController.hxx
    controller/customerController.hxx 
    controller/unitController.hxx
    controller/meterController.hxx
    controller/paymentController.hxx
    controller/companiesController.hxx
    controller/usersController.hxx

    middleware/jsonparser.hxx
    middleware/session_guard.hxx

    http/branchHttp.hxx
    http/common.hxx
    http/cusomerHttp.hxx
    http/unitHttp.hxx
    http/meterHttp.hxx
    http/billHttp.hxx
    http/paymentHttp.hxx
    http/employeeHttp.hxx
    http/meterReadHtpp.hxx

    # core/date_convert.hxx
)

include_directories(${CMAKE_SOURCE_DIR})
link_directories(/usr/local/lib)

# Set the path to your custom Boost installation
set(BOOST_ROOT "/home/ghost/Documents/boost")
set(CMAKE_PREFIX_PATH "/home/ghost/Documents/boost")

# Find Boost libraries
find_package(Boost 1.86 REQUIRED COMPONENTS 
    filesystem 
    system 
    thread 
    regex 
    serialization 
    date_time
    json
)

# Find OpenSSL
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)


add_executable(MyProject ${SOURCES})

# Find ODB libraries
find_package(PkgConfig REQUIRED)
pkg_check_modules(ODB REQUIRED libodb)
pkg_check_modules(ODB_PGSQL REQUIRED libodb-pgsql)
pkg_check_modules(ODB_MYSQL REQUIRED libodb-mysql)

# Link ODB libraries
target_link_libraries(MyProject PRIVATE
    ${ODB_LIBRARIES}
    ${ODB_PGSQL_LIBRARIES}
    ${ODB_MYSQL_LIBRARIES}
    ${ODB_BOOST_LIBRARIES}
)

# Include ODB directories
target_include_directories(MyProject PRIVATE
    ${ODB_INCLUDE_DIRS}
    ${ODB_PGSQL_INCLUDE_DIRS}
    ${ODB_MYSQL_INCLUDE_DIRS}
    ${ODB_BOOST_INCLUDE_DIRS}
)

# Link Boost libraries
target_link_libraries(MyProject PRIVATE
    Boost::filesystem
    Boost::system
    Boost::thread
    Boost::regex
    Boost::serialization
    Boost::json
)

# Link OpenSSL libraries
target_link_libraries(MyProject PRIVATE 
    OpenSSL::SSL 
    OpenSSL::Crypto
    CURL::libcurl
)
