#include "command.h"
#include "../uart_putc/UART_PUTCHAR.h"
#include <string.h>

// Maximum number of commands
#define MAX_COMMANDS 64

// Command registry
static Command command_registry[MAX_COMMANDS];
static int cmd_count = 0;

// Helper to find command by name
static int find_command(const char* name) {
    for (int i = 0; i < cmd_count; i++) {
        const char* cmd_name = command_registry[i].name;
        const char* n1 = cmd_name;
        const char* n2 = name;
        while (*n1 && *n2 && *n1 == *n2) {
            n1++;
            n2++;
        }
        if (*n1 == '\0' && (*n2 == '\0' || *n2 == ' ')) {
            return i;
        }
    }
    return -1;
}

// Command initialization
void command_init() {
    cmd_count = 0;
    memset(command_registry, 0, sizeof(command_registry));
}

// Register a new command
int command_register(const char* name, void (*callback)(const char*), bool enabled) {
    if (cmd_count >= MAX_COMMANDS) {
        return -1;
    }
    if (find_command(name) != -1) {
        return -2;
    }
    
    command_registry[cmd_count].name = name;
    command_registry[cmd_count].callback = callback;
    command_registry[cmd_count].enabled = enabled;
    cmd_count++;
    return 0;
}

// Execute a command from input string
int command_execute(const char* input) {
    if (!input || !input[0]) {
        return 0;
    }
    
    // Parse command name and arguments
    char cmd_name[MAX_COMMAND_NAME];
    char args[256];
    
    int i = 0;
    while (input[i] == ' ') i++;
    
    int j = 0;
    while (input[i] && input[i] != ' ' && j < MAX_COMMAND_NAME - 1) {
        cmd_name[j++] = input[i++];
    }
    cmd_name[j] = '\0';
    
    // Skip spaces to get to args
    while (input[i] == ' ') i++;
    
    // Copy rest as arguments
    int k = 0;
    while (input[i] && k < 255) {
        args[k++] = input[i++];
    }
    args[k] = '\0';
    
    int idx = find_command(cmd_name);
    if (idx == -1) {
        uart_puts("Unknown command: '");
        uart_puts(cmd_name);
        uart_puts("'. Type HELP for a list of commands.\r\n");
        return -1;
    }
    
    if (!command_registry[idx].enabled) {
        uart_puts("Command disabled: '");
        uart_puts(cmd_name);
        uart_puts("'\r\n");
        return -2;
    }
    
    // Execute the command
    command_registry[idx].callback(args);
    return 0;
}

// Get command by index
const Command* command_get_by_index(int index) {
    if (index < 0 || index >= cmd_count) {
        return NULL;
    }
    return &command_registry[index];
}

// Get total command count
int command_count() {
    return cmd_count;
}
