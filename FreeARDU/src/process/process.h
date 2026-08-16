#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Maximum number of processes
#define MAX_PROCESSES 8
#define MAX_PROCESS_NAME 32

// Process state
typedef enum {
    PROC_STATE_STOPPED = 0,
    PROC_STATE_RUNNING,
    PROC_STATE_PAUSED,
    PROC_STATE_ERROR
} ProcessState;

// Process structure
typedef struct {
    char name[MAX_PROCESS_NAME];
    ProcessState state;
    void (*entry_function)(void);
    bool is_manager;  // Manager process (doesn't get killed by system_start_mgr)
    int priority;
} Process;

// Process management functions
void process_init();
int process_create(const char* name, void (*entry)(void), int priority);
int process_kill(const char* name);
int process_start(const char* name);
int process_stop(const char* name);
void process_list();
ProcessState process_get_state(const char* name);
Process* process_get(const char* name);
int process_count();

// Get process by index
Process* process_get_by_index(int index);

// Kill all processes except manager
void process_kill_all_except_manager();

// Start all processes (used by system_start_mgr)
void process_start_all();

// Manager management
void system_start_mgr();  // Kill all non-manager, restart all
int register_manager(const char* name, void (*entry)(void));

#ifdef __cplusplus
}
#endif

#endif // PROCESS_H
