#include <extended_memory.h>

// ---------------------------------------------------------------------------
// is_address_good: checks if a pointer is valid (POSIX-based, experimental)
// ---------------------------------------------------------------------------
static sigjmp_buf jump_buffer;

static void signal_handler(int signum) {
    siglongjmp(jump_buffer, 1);
}

bool is_address_good(void *addr) {
    // Setup signal handler
    struct sigaction old_action, new_action;
    new_action.sa_handler = signal_handler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;
    sigaction(SIGSEGV, &new_action, &old_action);

    // Attempt to dereference
    if (sigsetjmp(jump_buffer, 1) == 0) {
        volatile char value = *((volatile char *)addr);
        (void)value;
        sigaction(SIGSEGV, &old_action, NULL); // Restore old handler
        return true;
    }

    // If we got here, we had a SIGSEGV
    sigaction(SIGSEGV, &old_action, NULL); // Restore old handler
    return false;
}

// ---------------------------------------------------------------------------
// Safe memory wrappers
// ---------------------------------------------------------------------------
void *safe_malloc(size_t size) {
    void *ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "Error: Failed to allocate %zu bytes of memory.\n", size);
        exit(EXIT_FAILURE);
    }
    memset(ptr, 0, size);
    return ptr;
}

void *safe_realloc(void *ptr, size_t size) {
    void *new_ptr = realloc(ptr, size);
    if (!new_ptr) {
        fprintf(stderr, "Error: Failed to reallocate %zu bytes of memory.\n", size);
        exit(EXIT_FAILURE);
    }
    return new_ptr;
}

void safe_free(void **ptr) {
    if (ptr && *ptr) {
        free(*ptr);
        *ptr = NULL;
    }
}

void memory_dump(void *addr, size_t size) {
    unsigned char *byte = (unsigned char *)addr;
    printf("Memory dump of %zu bytes starting at %p:\n", size, addr);
    for (size_t i = 0; i < size; ++i) {
        if (i % 16 == 0) {
            printf("\n%p: ", (void *)(byte + i));
        }
        printf("%02x ", byte[i]);
    }
    printf("\n");
}
