#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <unordered_map>
#include <atomic>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#include "scheduler.h"
#include "../common/protocol.h"

using namespace std;

// ---------------- WORKER STRUCT ----------------
struct WorkerInfo {
    int fd;
    time_t lastHeartbeat;
    int activeJobs = 0;
};

vector<WorkerInfo> workers;
mutex worker_mtx;

// ---------------- JOB TRACKING ----------------
unordered_map<int, int> jobToWorker;
mutex jobTrack_mtx;

// ---------------- METRICS ----------------
atomic<int> totalDispatched{0};
atomic<int> totalCompleted{0};
atomic<int> totalFailed{0};

// ---------------- SERVER ----------------
int startServer(int port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (sockaddr*)&address, sizeof(address));
    listen(server_fd, 5);

    cout << "Scheduler running on port " << port << endl;
    return server_fd;
}

// ---------------- HANDLE WORKER ----------------
void handleWorker(int worker_fd) {
    char buffer[1024];

    while (true) {
        memset(buffer, 0, sizeof(buffer));

        int valread = recv(worker_fd, buffer, sizeof(buffer), 0);

        if (valread <= 0) break;

        string msg(buffer, valread);

        // HEARTBEAT
        if (msg.find(HEARTBEAT_MSG) != string::npos) {
            lock_guard<mutex> lock(worker_mtx);
            for (auto &w : workers) {
                if (w.fd == worker_fd) {
                    w.lastHeartbeat = time(nullptr);
                }
            }
        }

        // DONE / FAILED
        if (msg.find(DONE_MSG) != string::npos ||
            msg.find(FAILED_MSG) != string::npos) {

            int jobId;
            sscanf(msg.c_str(), "%*s %d", &jobId);

            {
                lock_guard<mutex> lock(worker_mtx);
                for (auto &w : workers) {
                    if (w.fd == worker_fd) {
                        w.activeJobs--;
                    }
                }
            }

            if (msg.find(DONE_MSG) != string::npos) totalCompleted++;
            if (msg.find(FAILED_MSG) != string::npos) totalFailed++;

            cout << "Job " << jobId << " finished\n";
        }
    }

    cout << "Worker disconnected\n";
}

// ---------------- ACCEPT ----------------
void acceptWorkers(int server_fd) {
    while (true) {
        int new_socket = accept(server_fd, nullptr, nullptr);
        if (new_socket < 0) continue;

        {
            lock_guard<mutex> lock(worker_mtx);
            workers.push_back({new_socket, time(nullptr), 0});
            cout << "Worker connected. Total: " << workers.size() << endl;
        }

        thread(handleWorker, new_socket).detach();
    }
}

// ---------------- MONITOR ----------------
void monitorWorkers() {
    while (true) {
        Sleep(3000);

        lock_guard<mutex> lock(worker_mtx);

        auto now = time(nullptr);

        workers.erase(remove_if(workers.begin(), workers.end(),
            [&](WorkerInfo &w) {
                if (now - w.lastHeartbeat > 5) {
                    cout << "Worker DEAD: " << w.fd << endl;
                    closesocket(w.fd);
                    return true;
                }
                return false;
            }), workers.end());
    }
}

// ---------------- DISPATCH ----------------
void dispatchJobs(Scheduler &scheduler) {
    while (true) {
        Job job = scheduler.getJob();

        int worker_fd = -1;

        {
            lock_guard<mutex> lock(worker_mtx);

            if (workers.empty()) {
                Sleep(1000);
                scheduler.requeueJob(job);
                continue;
            }

            // 🔥 LEAST LOADED WORKER
            int bestIdx = -1;
            int minLoad = INT_MAX;

            for (int i = 0; i < workers.size(); i++) {
                if (workers[i].activeJobs < minLoad) {
                    minLoad = workers[i].activeJobs;
                    bestIdx = i;
                }
            }

            workers[bestIdx].activeJobs++;
            worker_fd = workers[bestIdx].fd;
        }

        {
            lock_guard<mutex> lock(jobTrack_mtx);
            jobToWorker[job.id] = worker_fd;
        }

        string msg = string(JOB_MSG) + " " +
                     to_string(job.id) + " " +
                     to_string(job.duration) + "\n";

        send(worker_fd, msg.c_str(), msg.size(), 0);

        totalDispatched++;

        cout << "Dispatched Job " << job.id << endl;
    }
}

// ---------------- STATS ----------------
void printStats() {
    while (true) {
        Sleep(3000);

        cout << "\n====== SYSTEM STATS ======\n";
        cout << "Workers: " << workers.size() << endl;
        cout << "Dispatched: " << totalDispatched << endl;
        cout << "Completed: " << totalCompleted << endl;
        cout << "Failed: " << totalFailed << endl;

        cout << "Worker Loads:\n";
        for (auto &w : workers) {
            cout << "FD " << w.fd << " -> " << w.activeJobs << " jobs\n";
        }

        cout << "=========================\n";
    }
}

// ---------------- MAIN ----------------
int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);

    Scheduler scheduler;

    int server_fd = startServer(8080);

    thread t1(acceptWorkers, server_fd);
    thread t2(monitorWorkers);
    thread t3(printStats);

    cout << "Waiting for workers...\n";
    Sleep(5000);

    for (int i = 1; i <= 10; i++) {
        scheduler.submitJob(Job(i, rand()%5, rand()%3 + 1));
    }

    thread dispatcher(dispatchJobs, ref(scheduler));

    t1.join();
    t2.join();
    t3.join();
    dispatcher.join();

    closesocket(server_fd);
    WSACleanup();
}