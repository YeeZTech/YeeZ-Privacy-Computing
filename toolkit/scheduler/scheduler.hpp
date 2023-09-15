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
            waiting_task_map = std::make_shared<std::map<
                    std::string, std::shared_ptr<tf::Taskflow>
                    >>();
            pending_task_map = std::make_shared<std::map<
                    std::string, std::shared_ptr<tf::Taskflow>
                    >>();
        }

    public:
        nlohmann::json add_taskgraph(const std::string& path)
        {
            std::ifstream file(path);
            nlohmann::json data = nlohmann::json::parse(file); 

            return data;
        }

    private:
        void resolve_taskgraph(const nlohmann::json& json)
        {
            TaskGraph_SP tg = std::make_shared<TaskGraph>();

            insert_taskgraph(json["name"], tg);
        }

        // deprecated
        void run_taskgraph(TaskGraph_SP tg)
        {
            std::unique_lock wtm_ul_(wtm_mutex_1_);
            std::unique_lock ptm_ul_(ptm_mutex_2_);
            dispatcher_->run(*tg).wait();
        }

        void run_taskgraph(const std::string& name) {
            {
                std::unique_lock wtm_ul_(wtm_mutex_1_);
                std::unique_lock ptm_ul_(ptm_mutex_2_);
                auto tg_it = waiting_task_map->find(name);
                pending_task_map->insert(std::make_pair(tg_it->first, tg_it->second));
            }

            {
                std::unique_lock ptm_ul_(ptm_mutex_2_);
                auto tg_it = pending_task_map->find(name);
                dispatcher_->run(*(tg_it->second)).wait();
                pending_task_map->erase(tg_it);
            }
        }

        void insert_taskgraph(std::string name, TaskGraph_SP tg)
        {
            std::unique_lock tm_ul_(wtm_mutex_1_);
            waiting_task_map->insert(
                    std::pair<std::string, TaskGraph_SP>(name, tg)
                    );
        }

    private:
        std::shared_ptr<std::map<
            std::string,
            TaskGraph_SP
            >> waiting_task_map;
        mutable std::shared_mutex wtm_mutex_1_;
        std::shared_ptr<std::map<
            std::string,
            TaskGraph_SP
            >> pending_task_map;
        mutable std::shared_mutex ptm_mutex_2_;

        std::unique_ptr<TaskDispatcher> dispatcher_;
    }; 
}

#endif //YEEZ_PRIVACY_COMPUTING_SCHEDULER_H