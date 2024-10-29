#pragma once
#include "core/database.hxx"
#include "core/server.hxx"
#include <boost/json.hpp>
#include "controllers/unitController.hxx"
#include "controllers/branchController.hxx"

#include <boost/shared_ptr.hpp>

class UnitHttp{
public:
    static boost::json::object unit_to_json(const boost::shared_ptr<unit> un) {
        if(un){
            return {
                {"unit_id", un->get_unit_id()},
                {"unit_unique", un->get_unit_unique()},
                {"unit_price", un->get_unit_price()},
                {"unit_reg_date", un->get_unit_reg_date()},
                {
                    "branch_name", un->get_unit_branch()->get_branch_name()
                },
            };
        }
        return {};
    }

    static boost::json::array units_to_json(const std::vector<boost::shared_ptr<unit>> units) {
        boost::json::array json_array;
        for (const auto& un : units) {
            json_array.emplace_back(unit_to_json(un));
        }
        return json_array;
    }

    static void get(
        std::shared_ptr<odb::mysql::database>  & handle,
        const http::request<http::string_body>& req, 
        http::response<http::string_body>& res
        )
    {
        if (req.method() == http::verb::get) {
            std::cout << "http::verb::get called" <<endl;

            auto units = UnitController::getAllUnits(handle);
            auto units_json = units_to_json(units);
            std::string jsonString = boost::json::serialize(units_json);

            res.set(http::field::content_type, "application/json");
            res.body() = jsonString;
            res.prepare_payload();
        }
    }
    
    static void get_unit_by_uiid(
        std::shared_ptr<odb::mysql::database>  & handle,
        const http::request<http::string_body>& req, 
        http::response<http::string_body>& res, 
        const std::unordered_map<std::string, std::string>& query_params
        )
    {
        auto uuid = query_params.find("uuid");
        if (uuid != query_params.end()) {
            boost::json::object response_json;

            // Get first customer data
            std::string unit_uuid = uuid->second;
            auto unit_d = UnitController::getUnitByUuid(handle, unit_uuid);
            auto unit_d_json = unit_to_json(unit_d);
            response_json["unit_data"] = unit_d_json;


            std::string jsonString = boost::json::serialize(response_json);

            res.set(http::field::content_type, "application/json");
            res.body() = jsonString;
            res.prepare_payload();
        }else{
            res.result(http::status::bad_request);
            res.set(http::field::content_type, "application/json");
            res.body() = R"({"error": "Bad Request."})";
            res.prepare_payload();
            return;
        }

    }

    static void post(
        std::shared_ptr<odb::mysql::database>& handle,
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res
    ) {
        if (req.method() == http::verb::post) {
            std::cout << "http::verb::post Unit called" << std::endl;
            boost::json::value parsedValue = boost::json::parse(req.body());

            if (!parsedValue.is_object()) {
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"error": "Invalid JSON format. Expected an object."})";
                res.prepare_payload();
                return;
            }

            boost::uuids::uuid uuid = boost::uuids::random_generator()();
            std::string unit_unique = boost::uuids::to_string(uuid);
            std::string unit_reg_date = getCurrentDate();

            const boost::json::object& jsonBody = parsedValue.as_object();
            std::string unit_price = safe_get_value<std::string>(jsonBody, "unit_price", "0.0");
            std::string unit_branch = safe_get_value<std::string>(jsonBody, "unit_branch", "Branch");

            auto branch_d = BranchController::getBranchByUiid(handle, unit_branch);
            if(!branch_d){ // this should not happen
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"error": "Failed to get branch"})";
                res.prepare_payload();
                return;
            }

            boost::shared_ptr<unit> unit_d = boost::make_shared<unit>(
                unit_unique,
                branch_d,
                unit_price,
                unit_reg_date
            );

            if (UnitController::createUnit(handle, unit_d)) {
                res.result(http::status::ok);
                res.body() = R"({"message": "Unit created successfully!"})";
            } else {
                res.result(http::status::bad_request);
                res.body() = R"({"error": "Failed to create unit"})";
            }
            res.prepare_payload();
        } else {
            res.result(http::status::bad_request);
            res.set(http::field::content_type, "application/json");
            res.body() = R"({"error": "Bad Request."})";
            res.prepare_payload();
        }
    }

    static void put(
        std::shared_ptr<odb::mysql::database>& handle,
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res
    ) {
        if (req.method() == http::verb::put) {
            std::cout << "http::verb::put Unit called" << std::endl;
            boost::json::value parsedValue = boost::json::parse(req.body());

            if (!parsedValue.is_object()) {
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"error": "Invalid JSON format. Expected an object."})";
                res.prepare_payload();
                return;
            }

            if (UnitController::updateUnit(handle, parsedValue)) {
                res.result(http::status::ok);
                res.body() = R"({"message": "Unit updated successfully!"})";
            } else {
                res.result(http::status::bad_request);
                res.body() = R"({"error": "Failed to update unit"})";
            }
            res.prepare_payload();
        } else {
            res.result(http::status::bad_request);
            res.set(http::field::content_type, "application/json");
            res.body() = R"({"error": "Bad Request."})";
            res.prepare_payload();
        }
    }

    static void delete_data(
        std::shared_ptr<odb::mysql::database>& handle,
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res,
        const std::unordered_map<std::string, std::string>& query_params
    ) {
        if (req.method() == http::verb::delete_) {
            std::cout << "http::verb::delete Unit called" << std::endl;
            auto uuid = query_params.find("uuid");
            if (uuid != query_params.end()) {
                std::string unit_unique = uuid->second;
                if (UnitController::deleteUnit(handle, unit_unique)) {
                    res.result(http::status::ok);
                    res.body() = R"({"message": "Unit deleted successfully!"})";
                } else {
                    res.result(http::status::bad_request);
                    res.body() = R"({"error": "Failed to delete unit"})";
                }
                res.prepare_payload();
            } else {
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"error": "Bad Request."})";
                res.prepare_payload();
            }
        } else {
            res.result(http::status::bad_request);
            res.set(http::field::content_type, "application/json");
            res.body() = R"({"error": "Bad Request."})";
            res.prepare_payload();
        }
    }

};