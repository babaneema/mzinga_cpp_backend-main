// #include "jwt-cpp/jwt.h"
#include "core/database.hxx"
#include "core/server.hxx"
#include "core/sms.hxx"
#include <boost/json.hpp>
#include <boost/thread.hpp>

#include "http/branchHttp.hxx"
#include "http/customerHttp.hxx"
#include "http/unitHttp.hxx"
#include "http/meterHttp.hxx"
#include "http/billHttp.hxx"
#include "http/employeeHttp.hxx"
#include "http/meterReadHtpp.hxx"
#include "http/paymentHttp.hxx" 
#include "http/authHttp.hxx" 
#include "http/common.hxx"



#include <boost/shared_ptr.hpp>

 using namespace server;
int main(int argc, char* argv[]) {
    // handle coarse 
    server::add_allowed_origin("http://127.0.0.1:5173"); 
    server::add_allowed_origin("http://localhost:5173"); 
    server::add_allowed_origin("https://backend.mzingamaji.co.tz"); 
    server::add_allowed_origin("https://admin.mzingamaji.co.tz"); 
    server::add_allowed_origin("https://mzingamaji.co.tz"); 
    server::add_allowed_origin("https://loganimaji.co.tz"); 

    database::load_connections();
    // Initialize the database and add persons http://localhost:5173
    // std::shared_ptr<odb::mysql::database>  handle = database::init_db();


    // sendSingleSms("255763096136", "Hello. Sunday Jabil", "test");
    // return 0;

    register_route("/api/v1/auth", [](const http::request<http::string_body>& req, 
                                        http::response<http::string_body>& res, 
                                        const std::unordered_map<std::string, std::string>& query_params
                                        ) {
       
        if(req.method() == http::verb::post){
            AuthHttp::login(req,res);
        }
     
    });

    register_route("/api/v1/branch", [](const http::request<http::string_body>& req, 
                                                        http::response<http::string_body>& res, 
                                                        const std::unordered_map<std::string, std::string>& query_params
         
                                                        ) {
        if (req.method() == http::verb::get){
            auto uuid = query_params.find("uuid");
            if(uuid != query_params.end()){
                BranchHttp::get_branch_by_uiid(req,res,query_params);
            }
            else{
                BranchHttp::get(req,res,query_params);
            }
        }
     
        if(req.method() == http::verb::post){
            BranchHttp::post(req,res);
        }
        
        if(req.method() == http::verb::put){
            BranchHttp::put(req,res);
        }

        if(req.method() == http::verb::delete_){
            BranchHttp::delete_data(req,res,query_params);
        }

    });

     register_route("/api/v1/unit", [](const http::request<http::string_body>& req, 
                                                    http::response<http::string_body>& res, 
                                                    const std::unordered_map<std::string, std::string>& query_params
                                                    ) {
        if (req.method() == http::verb::get){
            auto uuid = query_params.find("uuid");
            if(uuid != query_params.end()){
                UnitHttp::get_unit_by_uiid(req,res,query_params);
            }
            else{
                UnitHttp::get(req,res);
            }
        }

        if(req.method() == http::verb::post){
            UnitHttp::post(req,res);
        }
        
        if(req.method() == http::verb::put){
            UnitHttp::put(req,res);
        }

        if(req.method() == http::verb::delete_){
            UnitHttp::delete_data(req,res,query_params);
        }
    });
    
    register_route("/api/v1/customer", [](const http::request<http::string_body>& req, 
                                                        http::response<http::string_body>& res, 
                                                        const std::unordered_map<std::string, std::string>& query_params
                                                        ) {
        if (req.method() == http::verb::get){
            auto uuid = query_params.find("uuid");
            auto id = query_params.find("id");
            
            if(uuid != query_params.end()){
                CustomerHttp::get_uiid_full_data(req,res,query_params);
            }else if(id != query_params.end()){
                CustomerHttp::getCustomerById(req,res,query_params);
            }
            else{
                CustomerHttp::get(req,res);
            }
        }

        if(req.method() == http::verb::post){
            CustomerHttp::post(req,res);
        }
        
        if(req.method() == http::verb::put){
            CustomerHttp::put(req,res);
        }

        if(req.method() == http::verb::delete_){
            CustomerHttp::delete_data(req,res,query_params);
        }
    });

    register_route("/api/v1/meter", [](const http::request<http::string_body>& req, http::response<http::string_body>& res, 
                                                    const std::unordered_map<std::string, std::string>& query_params
                                                    ) {

        if (req.method() == http::verb::get){
            auto uuid = query_params.find("uuid");
            auto customer = query_params.find("customer");
            auto query = query_params.find("query");
            if( query != query_params.end()){
                MeterHttp::queryMeter(req,res,query_params);
            }
            else if(uuid != query_params.end()){
                MeterHttp::getByUuid(req,res,query_params);
            }
            else if(customer != query_params.end()){
                MeterHttp::getByCustomerUuid(req,res,query_params);
            }
            else{
                MeterHttp::get(req,res);
            }
        }

        if(req.method() == http::verb::post){
            MeterHttp::post(req,res);
        }
        
        if(req.method() == http::verb::put){
            MeterHttp::put(req,res);
        }

        if(req.method() == http::verb::delete_){
            MeterHttp::delete_data(req,res,query_params);
        }
        
    });

    register_route("/api/v1/customer", [](const http::request<http::string_body>& req, 
                                        http::response<http::string_body>& res, 
                                        const std::unordered_map<std::string, std::string>& query_params
                                        ) {
        if (req.method() == http::verb::get){
            auto uuid = query_params.find("uuid");
            auto report = query_params.find("report");

            if(uuid != query_params.end()){
                CustomerHttp::get_uiid_full_data(req,res,query_params);
            }
            else if(report != query_params.end()){
                CustomerHttp::getCustomerReport(req,res,query_params);
            }
            else{
                CustomerHttp::get(req,res);
            }
        }

        if(req.method() == http::verb::post){

            auto meter = query_params.find("meter");
            if(meter != query_params.end()){
                CustomerHttp::searchCustomerMeter(req,res);
            }else{
                CustomerHttp::post(req,res);
            }
        }

        if(req.method() == http::verb::put){
             CustomerHttp::put(req,res);
        }

        if(req.method() == http::verb::delete_){
            CustomerHttp::delete_data(req,res,query_params);
        }
    });

    register_route("/api/v1/meter-read", [](const http::request<http::string_body>& req, 
                                        http::response<http::string_body>& res, 
                                        const std::unordered_map<std::string, std::string>& query_params
                                        ) {
        if (req.method() == http::verb::get){
            auto uuid = query_params.find("uuid");
            if(uuid != query_params.end()){
                CustomerHttp::get_uiid_full_data(req,res,query_params);
            }
            else{
                MeterReadHttp::seachCustomerInformation(req,res,query_params);
            }
        }
    });

    register_route("/api/v1/bill", [](const http::request<http::string_body>& req, 
                                        http::response<http::string_body>& res, 
                                        const std::unordered_map<std::string, std::string>& query_params
                                        ) {
        if (req.method() == http::verb::get){
            auto uuid = query_params.find("uuid");
            auto query = query_params.find("query");
            if( query != query_params.end()){
                BillHttp::queryBill(req,res,query_params);
            }
            if(uuid != query_params.end()){
                BillHttp::getByUuid(req,res,query_params);
            }
            else{
                BillHttp::get(req,res,query_params);
            }
        }

        if(req.method() == http::verb::post){
            BillHttp::post(req,res);
        }


        if(req.method() == http::verb::delete_){
            BillHttp::delete_data(req,res,query_params);
        }

    });

    register_route("/api/v1/payment", [](const http::request<http::string_body>& req, 
                                        http::response<http::string_body>& res, 
                                        const std::unordered_map<std::string, std::string>& query_params
                                        ) {
        if(req.method() == http::verb::post){
            PaymentHttp::payBill(req,res);
        }

        if(req.method() == http::verb::delete_){
            PaymentHttp::delete_data(req,res,query_params);
        }
    });

    register_route("/api/v1/employee", [](const http::request<http::string_body>& req, 
                                        http::response<http::string_body>& res, 
                                        const std::unordered_map<std::string, std::string>& query_params
                                        ) {
        if (req.method() == http::verb::get){
            auto uuid = query_params.find("uuid");
            if(uuid != query_params.end()){
                EmployeeHttp::getByUuid(req,res,query_params);
            }
            else{
                EmployeeHttp::get(req,res);
            }
        }

        if(req.method() == http::verb::post){
            EmployeeHttp::post(req,res);
        }
     
    });
  
    auto const threads = std::max<int>(1, boost::thread::hardware_concurrency());
    start_server(threads,8080);
    
    return 0;
}