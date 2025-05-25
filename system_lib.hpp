#ifndef SYSTEM_LIB_HPP
#define SYSTEM_LIB_HPP

#include <string>

namespace SystemLib {

// Function to create a new process
int createProcess(const std::string& command);

// Function to execute a process
int executeProcess(int processId);

// Function to terminate a process
int terminateProcess(int processId);

} // namespace SystemLib

#endif // SYSTEM_LIB_HPP
