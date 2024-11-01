// #include "jwt-cpp/jwt.h"
#include "core/database.hxx"
#include "core/server.hxx"
#include "core/sms.hxx"
#include <boost/json.hpp>
#include <boost/thread.hpp>

#include "http/branchHttp.hxx"
#include "http/cusomerHttp.hxx"
#include "http/unitHttp.hxx"
#include "http/meterHttp.hxx"
#include "http/billHttp.hxx"
#include "http/employeeHttp.hxx"
#include "http/meterReadHtpp.hxx"
#include "http/paymentHttp.hxx"
#include "http/common.hxx"



#include <boost/shared_ptr.hpp>

 using namespace server;
int main(int argc, char* argv[]) {
    // Initialize the database and add persons http://localhost:5173
    std::shared_ptr<odb::mysql::database>  handle = database::init_db();
    server::add_allowed_origin("http://127.0.0.1:5173"); 
    server::add_allowed_origin("http://localhost:5173"); 
    server::add_allowed_origin("https://backend.mzingamaji.co.tz"); 
    server::add_allowed_origin("https://admin.mzingamaji.co.tz"); 
    server::add_allowed_origin("https://mzingamaji.co.tz"); 

    // sendSingleSms("255763096136", "Hello. Sunday Jabil");
    // $massage = 'MZINGA MAJi GOUP, Umefanya malipo ya shiling : '.$amount.' Deni ni '.$total_depts - $amount.' Asante';
    // $massage = 'MZINGA MAJI, Umetumia Units :  '.$new_unit.' Kutoka : '.$sum_unit_used.'. Hadi : '.$reading.' Deni lako la nyuma ni Tsh : '.$past_dept.' Jumla ya deni lako ni Tsh :'.$total_b;

    register_route("/api/v1/branch", [&handle](const http::request<http::string_body>& req, 
                                                        http::response<http::string_body>& res, 
                                                        const std::unordered_map<std::string, std::string>& query_params,
         
                                                        const std::string& session_id) {
        if (req.method() == http::verb::get){
            auto uuid = query_params.find("uuid");
            if(uuid != query_params.end()){
                BranchHttp::get_branch_by_uiid(handle,req,res,query_params);
            }
            else{
                BranchHttp::get(handle,req,res,query_params);
            }
            
        }
     
        if(req.method() == http::verb::post){
            BranchHttp::post(handle,req,res);
        }
        
        if(req.method() == http::verb::put){
            BranchHttp::put(handle,req,res);
        }

        if(req.method() == http::verb::delete_){
            BranchHttp::delete_data(handle,req,res,query_params);
        }

    });
    
    register_route("/api/v1/customer", [&handle](const http::request<http::string_body>& req, 
                                                        http::response<http::string_body>& res, 
                                                        const std::unordered_map<std::string, std::string>& query_params,
                                                        const std::string& session_id) {
        if (req.method() == http::verb::get){
            // TODO:
            // WORK ON ID ISSUES
            auto uuid = query_params.find("uuid");
            auto id = query_params.find("id");
            
            if(uuid != query_params.end()){
                CustomerHttp::get_uiid_full_data(handle,req,res,query_params);
            }else if(id != query_params.end()){
                CustomerHttp::getCustomerById(handle,req,res,query_params);
            }
            else{
                CustomerHttp::get(handle,req,res);
            }
        }

        if(req.method() == http::verb::post){
            CustomerHttp::post(handle,req,res);
        }
        
        if(req.method() == http::verb::put){
            CustomerHttp::put(handle,req,res);
        }

        if(req.method() == http::verb::delete_){
            CustomerHttp::delete_data(handle,req,res,query_params);
        }
    });

    register_route("/api/v1/unit", [&handle](const http::request<http::string_body>& req, 
                                                    http::response<http::string_body>& res, 
                                                    const std::unordered_map<std::string, std::string>& query_params,
                                                    const std::string& session_id) {
        if (req.method() == http::verb::get){
            auto uuid = query_params.find("uuid");
            if(uuid != query_params.end()){
                UnitHttp::get_unit_by_uiid(handle,req,res,query_params);
            }
            else{
                UnitHttp::get(handle,req,res);
            }
        }

        if(req.method() == http::verb::post){
            UnitHttp::post(handle,req,res);
        }
        
        if(req.method() == http::verb::put){
            UnitHttp::put(handle,req,res);
        }

        if(req.method() == http::verb::delete_){
            UnitHttp::delete_data(handle,req,res,query_params);
        }
    });

    

    register_route("/api/v1/meter", [&handle](const http::request<http::string_body>& req, http::response<http::string_body>& res, 
                                                    const std::unordered_map<std::string, std::string>& query_params,
                                                    const std::string& session_id) {

        if (req.method() == http::verb::get){
            auto uuid = query_params.find("uuid");
            auto customer = query_params.find("customer");
            auto query = query_params.find("query");
            if( query != query_params.end()){
                MeterHttp::queryMeter(handle,req,res,query_params);
            }
            else if(uuid != query_params.end()){
                MeterHttp::getByUuid(handle,req,res,query_params);
            }
            else if(customer != query_params.end()){
                MeterHttp::getByCustomerUuid(handle,req,res,query_params);
            }
            else{
                MeterHttp::get(handle,req,res);
            }
        }

        if(req.method() == http::verb::post){
            MeterHttp::post(handle,req,res);
        }
        
        if(req.method() == http::verb::put){
            MeterHttp::put(handle,req,res);
        }

        if(req.method() == http::verb::delete_){
            MeterHttp::delete_data(handle,req,res,query_params);
        }
        
    });

    register_route("/api/v1/customer", [&handle](const http::request<http::string_body>& req, 
                                        http::response<http::string_body>& res, 
                                        const std::unordered_map<std::string, std::string>& query_params,
                                        const std::string& session_id) {
        if (req.method() == http::verb::get){
            auto uuid = query_params.find("uuid");
            auto report = query_params.find("report");

            if(uuid != query_params.end()){
                CustomerHttp::get_uiid_full_data(handle,req,res,query_params);
            }
            else if(report != query_params.end()){
                CustomerHttp::getCustomerReport(handle,req,res,query_params);
            }
            else{
                CustomerHttp::get(handle,req,res);
            }
        }

        if(req.method() == http::verb::post){

            auto meter = query_params.find("meter");
            if(meter != query_params.end()){
                CustomerHttp::searchCustomerMeter(handle,req,res);
            }else{
                CustomerHttp::post(handle,req,res);
            }
        }

        if(req.method() == http::verb::put){
             CustomerHttp::put(handle,req,res);
        }

        if(req.method() == http::verb::delete_){
            CustomerHttp::delete_data(handle,req,res,query_params);
        }
    });

    register_route("/api/v1/meter-read", [&handle](const http::request<http::string_body>& req, 
                                        http::response<http::string_body>& res, 
                                        const std::unordered_map<std::string, std::string>& query_params,
                                        const std::string& session_id) {
        if (req.method() == http::verb::get){
            auto uuid = query_params.find("uuid");
            if(uuid != query_params.end()){
                CustomerHttp::get_uiid_full_data(handle,req,res,query_params);
            }
            else{
                MeterReadHttp::seachCustomerInformation(handle,req,res,query_params);
            }
        }
    });

    register_route("/api/v1/bill", [&handle](const http::request<http::string_body>& req, 
                                        http::response<http::string_body>& res, 
                                        const std::unordered_map<std::string, std::string>& query_params,
                                        const std::string& session_id) {
        if (req.method() == http::verb::get){
            auto uuid = query_params.find("uuid");
            if(uuid != query_params.end()){
                BillHttp::getByUuid(handle,req,res,query_params);
            }
            else{
                BillHttp::get(handle,req,res);
            }
        }

        if(req.method() == http::verb::post){
            BillHttp::post(handle,req,res);
        }


        if(req.method() == http::verb::delete_){
            BillHttp::delete_data(handle,req,res,query_params);
        }

    });

    register_route("/api/v1/payment", [&handle](const http::request<http::string_body>& req, 
                                        http::response<http::string_body>& res, 
                                        const std::unordered_map<std::string, std::string>& query_params,
                                        const std::string& session_id) {
        if(req.method() == http::verb::post){
            PaymentHttp::payBill(handle,req,res);
        }

        if(req.method() == http::verb::delete_){
            PaymentHttp::delete_data(handle,req,res,query_params);
        }
    });

    register_route("/api/v1/employee", [&handle](const http::request<http::string_body>& req, 
                                        http::response<http::string_body>& res, 
                                        const std::unordered_map<std::string, std::string>& query_params,
                                        const std::string& session_id) {
        if (req.method() == http::verb::get){
            auto uuid = query_params.find("uuid");
            if(uuid != query_params.end()){
                // BillHttp::getByUuid(handle,req,res,query_params);
            }
            else{
                EmployeeHttp::get(handle,req,res);
            }
        }

        if(req.method() == http::verb::post){
            EmployeeHttp::post(handle,req,res);
        }
     
    });
  
    auto const threads = std::max<int>(1, boost::thread::hardware_concurrency());
    start_server(threads);
    
    return 0;
}