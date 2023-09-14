#ifndef YEEZ_PRIVACY_COMPUTING_SCHEDULER_H
#define YEEZ_PRIVACY_COMPUTING_SCHEDULER_H

#include "dag.hpp"

#include <mutex>
#include <shared_mutex>

namespace cluster {

    typedef tf::Executor TaskDispatcher;
    typedef tf::Taskflow TaskGraph; 
    typedef std::shared_ptr<TaskGraph> TaskGraph_SP;  

    class Scheduler {
    public:
        Scheduler() {
            dispatcher_ = std::make_unique<TaskDispatcher>();
        }

    public:
        nlohmann::json add_taskgraph(const std::string& path)
        {
            std::ifstream file(path);
            nlohmann::json data = nlohmann::json::parse(file); 

            return data;
        }

        void resolve_taskgraph(const nlohmann::json& json)
        {
            TaskGraph_SP tg = std::make_shared<TaskGraph>();

            insert_taskgraph("", tg);
        }

        void run_taskgraph(TaskGraph_SP tg)
        {
            dispatcher_->run(*tg).wait();
        }

        void run_taskgraph(const std::string& name)
        {

        }

    private: 
        void insert_taskgraph(std::string name, TaskGraph_SP tg)
        {
            std::unique_lock tm_ul_(waiting_task_map_mutex_);
            waiting_task_map->insert(
                    std::pair<std::string, std::shared_ptr<tf::Taskflow>>(name, tg)
                    );
        }

    private:
        std::shared_ptr<std::map<
            std::string,
            std::shared_ptr<tf::Taskflow>
            >> waiting_task_map;
        mutable std::shared_mutex waiting_task_map_mutex_;
        std::shared_ptr<std::map<
                std::string,
                std::shared_ptr<tf::Taskflow>
        >> pending_task_map;
        mutable std::shared_mutex pending_task_map_mutex_;

        std::unique_ptr<TaskDispatcher> dispatcher_;
    }; 
}

#endif //YEEZ_PRIVACY_COMPUTING_SCHEDULER_H