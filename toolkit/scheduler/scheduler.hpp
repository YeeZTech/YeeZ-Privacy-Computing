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
        Scheduler();

    public:
        nlohmann::json add_taskgraph(const std::string& path);

    private:
        void resolve_taskgraph(const nlohmann::json& json);

        // deprecated
        void run_taskgraph(TaskGraph_SP tg);

        void run_taskgraph(const std::string& name);

        void insert_taskgraph(std::string name, TaskGraph_SP tg);

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