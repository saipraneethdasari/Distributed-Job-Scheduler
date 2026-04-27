#include "scheduler.h"

void Scheduler::submitJob(Job job) {
    {
        std::lock_guard<std::mutex> lock(job_mtx);
        jobMap[job.id] = job;
    }

    {
        std::lock_guard<std::mutex> lock(queue_mtx);
        jobQueue.push(job);
    }

    cv.notify_one();
}

Job Scheduler::getJob() {
    std::unique_lock<std::mutex> lock(queue_mtx);

    cv.wait(lock, [&]() { return !jobQueue.empty(); });

    Job job = jobQueue.top();
    jobQueue.pop();

    return job;
}

void Scheduler::requeueJob(Job job) {
    if (job.retries < 3) {
        job.retries++;
        job.status = JobStatus::QUEUED;

        submitJob(job);
    } else {
        job.status = JobStatus::FAILED;
    }
}