#ifndef COMMAND_H
#define COMMAND_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Maximum length for command name
#define MAX_COMMAND_NAME 32

// Command structure with void callback
typedef struct Command {
    const char* name;
    void (*callback)(const char* args);
    bool enabled;
} Command;

// Command registry functions
void command_init();
int command_register(const char* name, void (*callback)(const char*), bool enabled);
int command_execute(const char* input);
int command_list_all(char* buffer, int max_len);
int command_find(const char* name);

// Get command by index (for iteration)
const Command* command_get_by_index(int index);
int command_count();

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
// C++ callback namespace for command implementations
namespace CommandCallbacks {
    void help(const char* args);
    void version(const char* args);
    void uptime(const char* args);
    void cpuinfo(const char* args);
    void memstat(const char* args);
    void date(const char* args);
    void ls(const char* args);
    void echo(const char* args);
    void clear(const char* args);
    void crt(const char* args);
    void cd(const char* args);
    void rd(const char* args);
    void dump_memory(const char* args);
    void memtest(const char* args);
    void screen_info(const char* args);
    void stress_test(const char* args);
    void restart(const char* args);
    void halt(const char* args);
    void panic(const char* args);
    void cause_fault(const char* args);
    void and_op(const char* args);
    void kill_proc(const char* args);
    void lst_parc(const char* args);
    void system_start_mgr(const char* args);
    void bios_load(const char* args);
    void bios_set_param(const char* args);
    void bios_get(const char* args);
    void bios_cfg(const char* args);
}
#endif

#endif // COMMAND_H
