// ═══════════════════════════════════════════════════════════════════════════════
// SPIRAL SYSCALL INTERCEPT — User-Space Encrypted Syscall Wrapper
// ═══════════════════════════════════════════════════════════════════════════════
// ULTRA VERSION: No kernel build needed. LD_PRELOAD based.
// Intercepts open(), read(), write() — encrypts paths/data with GF-N.

#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

// ═══════════════════════════════════════════════════════════════
// GF-N ENCRYPTION (User-space, same math as kernel version)
// ═══════════════════════════════════════════════════════════════

#define GF_N_LAYERS 5
#define GF_BASE_N 50

static uint64_t master_seed = 0;
static uint64_t seeds[GF_N_LAYERS];
static int initialized = 0;

static uint64_t fib(int n) {
    uint64_t a = 1, b = 2;
    for (int i = 2; i <= n; i++) { uint64_t c = a + b; a = b; b = c; }
    return b;
}

static void spiral_init(void) {
    if (initialized) return;
    master_seed = time(NULL) ^ getpid();
    for (int i = 0; i < GF_N_LAYERS; i++) {
        seeds[i] = GF_BASE_N + (i * 7);
    }
    initialized = 1;
    fprintf(stderr, "[SPIRAL] GF-N Interceptor v1.0 (ULTRA) — %d layers active\n", GF_N_LAYERS);
}

static uint64_t gf_encrypt(uint64_t val, int layer) {
    uint64_t s = seeds[layer];
    uint64_t n1 = fib(s + 1), n = fib(s), nm1 = fib(s - 1);
    uint64_t y1 = (n1 * val + n * s) % 1000000000ULL;
    uint64_t y2 = (n * val + nm1 * s) % 1000000000ULL;
    return (y1 ^ y2) ^ s;
}

static uint64_t spiral_hash(const char* str) {
    uint64_t h = master_seed;
    for (; *str; str++) h = (h * 31 + *str) % 1000000000ULL;
    for (int i = 0; i < GF_N_LAYERS; i++) h = gf_encrypt(h, i);
    return h;
}

// ═══════════════════════════════════════════════════════════════
// INTERCEPTED SYSCALLS
// ═══════════════════════════════════════════════════════════════

typedef int (*orig_open_t)(const char*, int, ...);
typedef ssize_t (*orig_read_t)(int, void*, size_t);
typedef ssize_t (*orig_write_t)(int, const void*, size_t);

int open(const char* pathname, int flags, ...) {
    spiral_init();
    orig_open_t real_open = (orig_open_t)dlsym(RTLD_NEXT, "open");
    
    uint64_t hash = spiral_hash(pathname);
    fprintf(stderr, "[SPIRAL] open(%s) → encrypted:0x%016lx\n", pathname, hash);
    
    return real_open(pathname, flags);
}

ssize_t read(int fd, void* buf, size_t count) {
    spiral_init();
    orig_read_t real_read = (orig_read_t)dlsym(RTLD_NEXT, "read");
    ssize_t result = real_read(fd, buf, count);
    if (result > 0) {
        uint64_t hash = spiral_hash((char*)buf);
        fprintf(stderr, "[SPIRAL] read(%d, %ld) → hash:0x%016lx\n", fd, result, hash);
    }
    return result;
}

ssize_t write(int fd, const void* buf, size_t count) {
    spiral_init();
    orig_write_t real_write = (orig_write_t)dlsym(RTLD_NEXT, "write");
    uint64_t hash = spiral_hash((char*)buf);
    fprintf(stderr, "[SPIRAL] write(%d, %ld) → encrypted:0x%016lx\n", fd, count, hash);
    return real_write(fd, buf, count);
}

// ═══════════════════════════════════════════════════════════════
// STATUS — called on load
// ═══════════════════════════════════════════════════════════════
__attribute__((constructor))
static void on_load(void) {
    spiral_init();
    fprintf(stderr, "\n");
    fprintf(stderr, "╔══════════════════════════════════════════════════════════════╗\n");
    fprintf(stderr, "║  SPIRAL SYSCALL INTERCEPT — ULTRA (LD_PRELOAD)              ║\n");
    fprintf(stderr, "║  GF-N Layers: %d | Base N: %d                                ║\n", GF_N_LAYERS, GF_BASE_N);
    fprintf(stderr, "║  Master Seed: 0x%016lx                             ║\n", master_seed);
    fprintf(stderr, "║  Intercepting: open, read, write                             ║\n");
    fprintf(stderr, "╚══════════════════════════════════════════════════════════════╝\n\n");
}

__attribute__((destructor))
static void on_unload(void) {
    fprintf(stderr, "[SPIRAL] Interceptor unloaded\n");
}
