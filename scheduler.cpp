// Basic example of a process scheduler in C++
#include <iostream>
#include <vector>

class Process {
public:
    int id;
    int burstTime;
    int waitingTime;
    int turnaroundTime;

    Process(int id, int burstTime) : id(id), burstTime(burstTime), waitingTime(0), turnaroundTime(0) {}
};

class Scheduler {
public:
    std::vector<Process> processes;

    void addProcess(Process p) {
        processes.push_back(p);
    }

    void schedule() {
        // Simple First-Come-First-Served (FCFS) scheduling algorithm
        for (auto& p : processes) {
            std::cout << "Process " << p.id << " is running." << std::endl;
            // Simulate execution
            p.turnaroundTime = p.burstTime;
            std::cout << "Process " << p.id << " finished. Turnaround Time: " << p.turnaroundTime << std::endl;
        }
    }
};

int main() {
    Scheduler scheduler;
    scheduler.addProcess(Process(1, 5));
    scheduler.addProcess(Process(2, 3));
    scheduler.addProcess(Process(3, 1));
    scheduler.schedule();
    return 0;
}