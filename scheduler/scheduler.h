#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <queue>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "../common/job.h"

struct Compare {
    bool operator()(Job const& a, Job const& b) {
        return a.priority < b.priority;
    }
};

class Scheduler {
public:
    void submitJob(Job job);
    Job getJob();
    void requeueJob(Job job);

private:
    std::priority_queue<Job, std::vector<Job>, Compare> jobQueue;
    std::unordered_map<int, Job> jobMap;

    std::mutex queue_mtx;
    std::mutex job_mtx;
    std::condition_variable cv;
};

#endif