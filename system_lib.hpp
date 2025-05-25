// SystemLib.hpp - Header file
#ifndef SYSTEM_LIB_HPP
#define SYSTEM_LIB_HPP

#include <string>
#include <vector>
#include <memory>

namespace SystemLib {

// Error codes
enum class SystemError {
    SUCCESS = 0,
    INVALID_COMMAND = -1,
    PROCESS_NOT_FOUND = -2,
    EXECUTION_FAILED = -3,
    PERMISSION_DENIED = -4,
    RESOURCE_UNAVAILABLE = -5
};

// Process information structure
struct ProcessInfo {
    int processId;
    std::string command;
    bool isRunning;
    int exitCode;
};

// Main API functions
SystemError createProcess(const std::string& command, int& processId);
SystemError executeProcess(int processId);
SystemError terminateProcess(int processId);
SystemError getProcessInfo(int processId, ProcessInfo& info);
SystemError listProcesses(std::vector<ProcessInfo>& processes);

// Utility functions
std::string getErrorMessage(SystemError error);
bool isValidCommand(const std::string& command);

} // namespace SystemLib

#endif // SYSTEM_LIB_HPP

// SystemLib.cpp - Implementation
#include "SystemLib.hpp"
#include <unordered_map>
#include <mutex>
#include <thread>
#include <atomic>
#include <sstream>

#ifdef _WIN32
    #include <windows.h>
    #include <process.h>
#else
    #include <unistd.h>
    #include <sys/wait.h>
    #include <signal.h>
    #include <spawn.h>
#endif

namespace SystemLib {

// Internal process management
class ProcessManager {
private:
    static std::atomic<int> nextProcessId;
    static std::unordered_map<int, std::unique_ptr<ProcessInfo>> processes;
    static std::mutex processMutex;

public:
    static int generateProcessId() {
        return ++nextProcessId;
    }

    static void addProcess(int id, std::unique_ptr<ProcessInfo> info) {
        std::lock_guard<std::mutex> lock(processMutex);
        processes[id] = std::move(info);
    }

    static ProcessInfo* getProcess(int id) {
        std::lock_guard<std::mutex> lock(processMutex);
        auto it = processes.find(id);
        return (it != processes.end()) ? it->second.get() : nullptr;
    }

    static void removeProcess(int id) {
        std::lock_guard<std::mutex> lock(processMutex);
        processes.erase(id);
    }

    static std::vector<ProcessInfo> getAllProcesses() {
        std::lock_guard<std::mutex> lock(processMutex);
        std::vector<ProcessInfo> result;
        for (const auto& pair : processes) {
            result.push_back(*pair.second);
        }
        return result;
    }
};

// Static member definitions
std::atomic<int> ProcessManager::nextProcessId{1000};
std::unordered_map<int, std::unique_ptr<ProcessInfo>> ProcessManager::processes;
std::mutex ProcessManager::processMutex;

// Utility function implementations
bool isValidCommand(const std::string& command) {
    if (command.empty()) return false;
    
    // Basic validation - check for dangerous patterns
    std::vector<std::string> dangerous = {";", "|", "&", ">", "<", "`", "$("};
    for (const auto& pattern : dangerous) {
        if (command.find(pattern) != std::string::npos) {
            return false;
        }
    }
    return true;
}

std::string getErrorMessage(SystemError error) {
    switch (error) {
        case SystemError::SUCCESS:
            return "Operation completed successfully";
        case SystemError::INVALID_COMMAND:
            return "Invalid or unsafe command";
        case SystemError::PROCESS_NOT_FOUND:
            return "Process not found";
        case SystemError::EXECUTION_FAILED:
            return "Process execution failed";
        case SystemError::PERMISSION_DENIED:
            return "Permission denied";
        case SystemError::RESOURCE_UNAVAILABLE:
            return "System resources unavailable";
        default:
            return "Unknown error";
    }
}

// Cross-platform process creation
#ifdef _WIN32
SystemError createProcessImpl(const std::string& command, int processId) {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Create the process in suspended state
    BOOL success = CreateProcessA(
        NULL,                           // Application name
        const_cast<char*>(command.c_str()), // Command line
        NULL,                           // Process security attributes
        NULL,                           // Thread security attributes
        FALSE,                          // Inherit handles
        CREATE_SUSPENDED,               // Creation flags
        NULL,                           // Environment
        NULL,                           // Current directory
        &si,                           // Startup info
        &pi                            // Process info
    );

    if (!success) {
        return SystemError::EXECUTION_FAILED;
    }

    // Store process handles (in real implementation, you'd store these properly)
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    return SystemError::SUCCESS;
}
#else
SystemError createProcessImpl(const std::string& command, int processId) {
    // Parse command into arguments
    std::istringstream iss(command);
    std::vector<std::string> args;
    std::string arg;
    
    while (iss >> arg) {
        args.push_back(arg);
    }
    
    if (args.empty()) {
        return SystemError::INVALID_COMMAND;
    }
    
    // Convert to char* array for execv
    std::vector<char*> argv;
    for (auto& arg : args) {
        argv.push_back(const_cast<char*>(arg.c_str()));
    }
    argv.push_back(nullptr);
    
    // For now, just validate - actual fork/exec would happen in executeProcess
    if (access(args[0].c_str(), X_OK) != 0) {
        return SystemError::EXECUTION_FAILED;
    }
    
    return SystemError::SUCCESS;
}
#endif

// Main API implementations
SystemError createProcess(const std::string& command, int& processId) {
    if (!isValidCommand(command)) {
        return SystemError::INVALID_COMMAND;
    }
    
    processId = ProcessManager::generateProcessId();
    
    auto processInfo = std::make_unique<ProcessInfo>();
    processInfo->processId = processId;
    processInfo->command = command;
    processInfo->isRunning = false;
    processInfo->exitCode = 0;
    
    SystemError result = createProcessImpl(command, processId);
    if (result == SystemError::SUCCESS) {
        ProcessManager::addProcess(processId, std::move(processInfo));
    }
    
    return result;
}

SystemError executeProcess(int processId) {
    ProcessInfo* info = ProcessManager::getProcess(processId);
    if (!info) {
        return SystemError::PROCESS_NOT_FOUND;
    }
    
    if (info->isRunning) {
        return SystemError::SUCCESS; // Already running
    }
    
    // Mark as running
    info->isRunning = true;
    
    // In a real implementation, this would actually execute the process
    // For demonstration, we'll simulate execution in a separate thread
    std::thread([processId]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        ProcessInfo* info = ProcessManager::getProcess(processId);
        if (info) {
            info->isRunning = false;
            info->exitCode = 0; // Simulate successful completion
        }
    }).detach();
    
    return SystemError::SUCCESS;
}

SystemError terminateProcess(int processId) {
    ProcessInfo* info = ProcessManager::getProcess(processId);
    if (!info) {
        return SystemError::PROCESS_NOT_FOUND;
    }
    
    if (!info->isRunning) {
        return SystemError::SUCCESS; // Already terminated
    }
    
    // In real implementation, would use kill() on Unix or TerminateProcess() on Windows
    info->isRunning = false;
    info->exitCode = -1; // Terminated
    
    return SystemError::SUCCESS;
}

SystemError getProcessInfo(int processId, ProcessInfo& info) {
    ProcessInfo* processInfo = ProcessManager::getProcess(processId);
    if (!processInfo) {
        return SystemError::PROCESS_NOT_FOUND;
    }
    
    info = *processInfo;
    return SystemError::SUCCESS;
}

SystemError listProcesses(std::vector<ProcessInfo>& processes) {
    processes = ProcessManager::getAllProcesses();
    return SystemError::SUCCESS;
}

} // namespace SystemLib

// Example usage
#include <iostream>

int main() {
    using namespace SystemLib;
    
    int processId;
    SystemError result;
    
    // Create a process
    result = createProcess("echo Hello World", processId);
    if (result == SystemError::SUCCESS) {
        std::cout << "Process created with ID: " << processId << std::endl;
        
        // Execute the process
        result = executeProcess(processId);
        if (result == SystemError::SUCCESS) {
            std::cout << "Process executed successfully" << std::endl;
            
            // Get process information
            ProcessInfo info;
            if (getProcessInfo(processId, info) == SystemError::SUCCESS) {
                std::cout << "Process ID: " << info.processId << std::endl;
                std::cout << "Command: " << info.command << std::endl;
                std::cout << "Running: " << (info.isRunning ? "Yes" : "No") << std::endl;
                std::cout << "Exit Code: " << info.exitCode << std::endl;
            }
        }
    } else {
        std::cout << "Error: " << getErrorMessage(result) << std::endl;
    }
    
    return 0;
}
