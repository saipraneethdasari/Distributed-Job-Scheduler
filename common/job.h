#ifndef JOB_H
#define JOB_H

#include <chrono>

enum class JobStatus {
    QUEUED,
    RUNNING,
    COMPLETED,
    FAILED
};

struct Job {
    int id;
    int priority;
    int duration;
    int retries;
    JobStatus status;
    std::chrono::steady_clock::time_point submitTime;

    // Default constructor
    Job() : id(0), priority(0), duration(0),
            retries(0), status(JobStatus::QUEUED),
            submitTime(std::chrono::steady_clock::now()) {}

    // Parameterized
    Job(int i, int p, int d)
        : id(i), priority(p), duration(d),
          retries(0), status(JobStatus::QUEUED),
          submitTime(std::chrono::steady_clock::now()) {}
};

#endif