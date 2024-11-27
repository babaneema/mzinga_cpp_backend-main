#pragma once
#include <boost/json.hpp>
#include <memory>
#include <vector>
#include <odb/transaction.hxx>
#include <odb/query.hxx>
#include <odb/result.hxx>
#include <boost/shared_ptr.hpp>
#include "../models/tasks.hxx"
#include "../models/tasks-odb.hxx"
#include "core/safe_json.hxx"
#include "core/helpers.hxx"

class TaskController {
public:
    typedef odb::query<Task> query;
    typedef odb::result<Task> result;

    static auto getAllTasks(const std::shared_ptr<odb::mysql::database>& db) {
        std::vector<boost::shared_ptr<Task>> tasks;
        try {
            odb::transaction t(db->begin());
            odb::result<Task> r(db->query<Task>());
            for (auto i = r.begin(); i != r.end(); ++i) {
                tasks.emplace_back(boost::make_shared<Task>(*i));
            }
            t.commit();
            return tasks;
        } catch (const std::exception& e) {
            logger("TaskController::getAllTasks", e.what());
            std::cerr << "Error fetching tasks: " << e.what() << std::endl;
            return tasks;
        }
    }

    static std::vector<boost::shared_ptr<Task>> getPayRoll(
        const std::shared_ptr<odb::mysql::database>& db, 
        const std::string& employee_id, 
        const std::string& start, 
        const std::string& end) 
    {
        using namespace odb::core;
        std::vector<boost::shared_ptr<Task>> tasks;

        try {
            std::string query = R"(
                SELECT 
                    t.task_id,
                    t.task_unique,
                    t.task_item,
                    t.task_amount,
                    t.task_start,
                    t.task_end,
                    t.task_reg_date,
                    e.employee_name,
                    e.employee_contact
                FROM Task t
                LEFT JOIN employee e ON t.task_employee = e.employee_id
                WHERE e.employee_id = ?
                AND STR_TO_DATE(t.task_start, '%Y-%m-%d') >= STR_TO_DATE(?, '%Y-%m-%d')
                AND STR_TO_DATE(t.task_end, '%Y-%m-%d') <= STR_TO_DATE(?, '%Y-%m-%d')
                ORDER BY t.task_start
            )";
        }
        catch (const odb::exception& e) {
            // Handle exceptions
            std::cerr << "Error fetching tasks: " << e.what() << std::endl;
        }

        return tasks;
    }

    static boost::shared_ptr<Task> getTaskById(const std::shared_ptr<odb::mysql::database>& db, const unsigned long& id) {
        try {
            boost::shared_ptr<Task> task_data = boost::make_shared<Task>();
            odb::transaction t(db->begin());
            db->load(id, *task_data);
            t.commit();
            return task_data;
        } catch (const std::exception& e) {
            logger("TaskController::getTaskById", e.what());
            std::cerr << "Error fetching task by ID: " << e.what() << std::endl;
            return nullptr;
        }
    }

    static boost::shared_ptr<Task> getTaskByUuid(const std::shared_ptr<odb::mysql::database>& db, const std::string& uuid) {
        try {
            odb::transaction t(db->begin());
            boost::shared_ptr<Task> task_data(
                db->query_one<Task>(query::task_unique == uuid));
            t.commit();
            return task_data;
        } catch (const std::exception& e) {
            logger("TaskController::getTaskByUuid", e.what());
            std::cerr << "Error fetching task by UUID: " << e.what() << std::endl;
            return nullptr;
        }
    }

    static bool createTask(const std::shared_ptr<odb::mysql::database>& db, boost::shared_ptr<Task> task_d) {
        try {
            odb::transaction t(db->begin());
            db->persist(task_d);
            t.commit();
            return true;
        } catch (const std::exception& e) {
            logger("TaskController::createTask", e.what());
            std::cerr << "Error creating task: " << e.what() << std::endl;
            return false;
        }
    }

    static auto updateTask(const std::shared_ptr<odb::mysql::database>& db, const boost::json::value& values) {
        try {
            const boost::json::object& jsonBody = values.as_object();

            std::string task_uuid = safe_get_value<std::string>(jsonBody, "task_unique", "Task UUID");
            std::string task_item = safe_get_value<std::string>(jsonBody, "task_item", "Task Item");
            std::string task_amount = safe_get_value<std::string>(jsonBody, "task_amount", "Task Amount");

            auto task_d = getTaskByUuid(db, task_uuid);
            if (!task_d) {
                return false;
            }

            task_d->set_task_item(task_item);
            task_d->set_task_amount(task_amount);

            odb::transaction t(db->begin());
            db->update(task_d);
            t.commit();
            return true;

        } catch (const std::exception& e) {
            logger("TaskController::updateTask", e.what());
            std::cerr << "Error updating task: " << e.what() << std::endl;
            return false;
        }
    }

    static auto deleteTask(const std::shared_ptr<odb::mysql::database>& db, const std::string& uuid) {
        try {
            auto task_d = getTaskByUuid(db, uuid);
            if (!task_d) {
                std::cout << "Could not find task" << std::endl;
                return false;
            }

            odb::transaction t(db->begin());
            db->erase(task_d);
            t.commit();
            return true;
        } catch (const std::exception& e) {
            logger("TaskController::deleteTask", e.what());
            std::cerr << "Error deleting task: " << e.what() << std::endl;
            return false;
        }
    }
};
