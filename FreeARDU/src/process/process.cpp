#include "process.h"
#include "../uart_putc/UART_PUTCHAR.h"
#include <string.h>

// Process table
static Process processes[MAX_PROCESSES];
static int proc_count = 0;
static Process* manager_process = NULL;

void process_init() {
    proc_count = 0;
    manager_process = NULL;
    memset(processes, 0, sizeof(processes));
}

int process_create(const char* name, void (*entry)(void), int priority) {
    if (proc_count >= MAX_PROCESSES) {
        return -1;
    }
    
    // Check for duplicate name
    for (int i = 0; i < proc_count; i++) {
        if (strcmp(processes[i].name, name) == 0) {
            return -2;
        }
    }
    
    Process* p = &processes[proc_count];
    strncpy(p->name, name, MAX_PROCESS_NAME - 1);
    p->name[MAX_PROCESS_NAME - 1] = '\0';
    p->state = PROC_STATE_STOPPED;
    p->entry_function = entry;
    p->is_manager = false;
    p->priority = priority;
    
    proc_count++;
    return 0;
}

int process_kill(const char* name) {
    for (int i = 0; i < proc_count; i++) {
        if (strcmp(processes[i].name, name) == 0) {
            // Don't allow killing the manager through this function
            if (processes[i].is_manager) {
                uart_puts("Cannot kill manager process directly.\r\n");
                return -1;
            }
            processes[i].state = PROC_STATE_STOPPED;
            uart_puts("Process '");
            uart_puts(name);
            uart_puts("' killed.\r\n");
            return 0;
        }
    }
    return -1;
}

int process_start(const char* name) {
    Process* p = process_get(name);
    if (!p) {
        return -1;
    }
    
    if (p->state == PROC_STATE_ERROR) {
        uart_puts("Cannot start process in ERROR state. Reset first.\r\n");
        return -1;
    }
    
    p->state = PROC_STATE_RUNNING;
    
    if (p->entry_function) {
        uart_puts("Starting process: ");
        uart_puts(name);
        uart_puts("\r\n");
        p->entry_function();
    }
    
    return 0;
}

int process_stop(const char* name) {
    Process* p = process_get(name);
    if (!p) {
        return -1;
    }
    
    p->state = PROC_STATE_STOPPED;
    uart_puts("Process '");
    uart_puts(name);
    uart_puts("' stopped.\r\n");
    return 0;
}

void process_list() {
    uart_puts("Processes:\r\n");
    uart_puts("ID  Name              State    Manager  Priority\r\n");
    uart_puts("--- ----------------- -------- -------- --------\r\n");
    
    for (int i = 0; i < proc_count; i++) {
        Process* p = &processes[i];
        const char* state_str = "STOPPED";
        switch (p->state) {
            case PROC_STATE_RUNNING: state_str = "RUNNING"; break;
            case PROC_STATE_PAUSED:  state_str = "PAUSED";  break;
            case PROC_STATE_ERROR:   state_str = "ERROR";   break;
        }
        
        uart_print_uint(i);
        uart_puts("  ");
        
        // Pad name
        uart_puts(p->name);
        int name_len = strlen(p->name);
        for (int j = 0; j < 17 - name_len; j++) uart_puts(" ");
        
        uart_puts(state_str);
        int state_len = strlen(state_str);
        for (int j = 0; j < 9 - state_len; j++) uart_puts(" ");
        
        uart_puts(p->is_manager ? "YES     " : "NO      ");
        uart_print_uint(p->priority);
        uart_puts("\r\n");
    }
}

ProcessState process_get_state(const char* name) {
    Process* p = process_get(name);
    if (!p) return PROC_STATE_STOPPED;
    return p->state;
}

Process* process_get(const char* name) {
    for (int i = 0; i < proc_count; i++) {
        if (strcmp(processes[i].name, name) == 0) {
            return &processes[i];
        }
    }
    return NULL;
}

int process_count() {
    return proc_count;
}

Process* process_get_by_index(int index) {
    if (index < 0 || index >= proc_count) {
        return NULL;
    }
    return &processes[index];
}

void process_kill_all_except_manager() {
    uart_puts("Killing all processes except manager...\r\n");
    for (int i = 0; i < proc_count; i++) {
        if (!processes[i].is_manager && processes[i].state == PROC_STATE_RUNNING) {
            processes[i].state = PROC_STATE_STOPPED;
            uart_puts("Killed: ");
            uart_puts(processes[i].name);
            uart_puts("\r\n");
        }
    }
}

void process_start_all() {
    uart_puts("Starting all processes...\r\n");
    for (int i = 0; i < proc_count; i++) {
        if (processes[i].state != PROC_STATE_RUNNING) {
            if (processes[i].entry_function) {
                processes[i].state = PROC_STATE_RUNNING;
                uart_puts("Started: ");
                uart_puts(processes[i].name);
                uart_puts("\r\n");
                processes[i].entry_function();
            }
        }
    }
}

int register_manager(const char* name, void (*entry)(void)) {
    if (proc_count >= MAX_PROCESSES) {
        return -1;
    }
    
    // Check if manager already registered
    if (manager_process != NULL) {
        return -2;
    }
    
    Process* p = &processes[proc_count];
    strncpy(p->name, name, MAX_PROCESS_NAME - 1);
    p->name[MAX_PROCESS_NAME - 1] = '\0';
    p->state = PROC_STATE_STOPPED;
    p->entry_function = entry;
    p->is_manager = true;
    p->priority = 0;  // Manager has highest priority
    
    manager_process = p;
    proc_count++;
    return 0;
}

void system_start_mgr() {
    if (!manager_process) {
        uart_puts("No manager process registered.\r\n");
        return;
    }
    
    uart_puts("=== SYSTEM START MGR ===\r\n");
    uart_puts("Manager: ");
    uart_puts(manager_process->name);
    uart_puts("\r\n");
    
    // Kill all non-manager processes
    process_kill_all_except_manager();
    
    // Restart manager (reset its state and restart)
    uart_puts("Restarting manager process...\r\n");
    manager_process->state = PROC_STATE_STOPPED;
    manager_process->state = PROC_STATE_RUNNING;
    
    if (manager_process->entry_function) {
        manager_process->entry_function();
    }
    
    // Start all other stopped processes
    process_start_all();
    
    uart_puts("=== SYSTEM START MGR COMPLETE ===\r\n");
}
