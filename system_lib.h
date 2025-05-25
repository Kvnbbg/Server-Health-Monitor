#ifndef SYSTEM_LIB_H
#define SYSTEM_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

// Function to create a new process
int create_process(const char* command);

// Function to execute a process
int execute_process(int process_id);

// Function to terminate a process
int terminate_process(int process_id);

#ifdef __cplusplus
}
#endif

#endif // SYSTEM_LIB_H
