#include <util.h>

void s_panic(const char* caller, const char *msg) {
    printf("PANIC (from %s): %s\n",caller,  msg);
    exit(EXIT_FAILURE);
}


// uint64_t get_memory_usage() {
//     #ifdef _WIN32
//     PROCESS_MEMORY_COUNTERS_EX pmc;
//     if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
//         return pmc.WorkingSetSize / 1024;
//     }
//     printf("Failed to get memory usage.\n");
//     return -1;

//     #elif defined(__APPLE__)
//     struct task_basic_info info;
//     mach_msg_type_number_t info_count = TASK_BASIC_INFO_COUNT;

//     if (task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&info, &info_count) != KERN_SUCCESS) {
//         printf("Failed to get memory usage.\n");
//         return -1;
//     }

//     printf("Memory usage: %lu KB\n", info.resident_size / 1024);
//     return info.resident_size / 1024;

//     #elif defined(__linux__)
//     FILE* file = fopen("/proc/self/statm", "r");
//     if (!file) {
//         perror("fopen");
//         return;
//     }

//     unsigned long size;
//     unsigned long resident;
//     unsigned long share;
//     unsigned long text;
//     unsigned long lib;
//     unsigned long data;
//     unsigned long dt;

//     if (fscanf(file, "%lu %lu %lu %lu %lu %lu %lu", &size, &resident, &share, &text, &lib, &data, &dt) != 7) {
//         perror("fscanf");
//         fclose(file);
//         return;
//     }

//     fclose(file);

//     long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024;
//     long resident_set_size_kb = resident * page_size_kb;

//     printf("Memory usage: %lu KB\n", resident_set_size_kb);
//     return resident_set_size_kb;

//     #endif
// }