#pragma once
#include "../core/constants.h"
#include "../utils/safe_math.h"
#include "../utils/logger.h"
#include <string>
#include <sstream>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstring>
#include <cstdint>
#include <unistd.h>
#include <iomanip>

// ═══════════════════════════════════════════════════════════════════════════════
// HARDWARE SENTINEL — Auto-Detection + Post-Quantum Entropy Sources
// ═══════════════════════════════════════════════════════════════════════════════
//
// Auto-detects hardware capabilities and provides hardware-sourced entropy
// for post-quantum obfuscation.
//
// Entropy Sources:
//   - RDTSC (CPU cycle counter noise)
//   - Cache timing (L1/L2/L3 access jitter)
//   - Thread jitter (multi-core race conditions)
//   - Memory patterns (RAM access timing)
//
// ═══════════════════════════════════════════════════════════════════════════════

struct HardwareCapabilities {
    int max_ring_dim;        // Maximum RingDim based on RAM
    int max_ckks_depth;      // Maximum CKKS depth
    int cpu_cores;           // Number of CPU threads
    size_t total_ram_gb;     // Total RAM in GB
    bool has_avx2;           // AVX2 support
    bool has_avx512;         // AVX-512 support
    bool has_aes_ni;         // AES-NI hardware acceleration
    std::string cpu_brand;   // CPU model name
    std::string os_name;     // Operating system

    static HardwareCapabilities detect();

    std::string report();
};

struct HardwareEntropy {
    // ═══════════════════════════════════════════════════════════
    // RDTSC — CPU timestamp counter (nanosecond noise)
    // ═══════════════════════════════════════════════════════════
    static uint64_t rdtsc_noise() {
        uint32_t lo, hi;
        __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
        return ((uint64_t)hi << 32) | lo;
    }

    // ═══════════════════════════════════════════════════════════
    // Cache noise — Timing jitter from cache line bouncing
    // ═══════════════════════════════════════════════════════════
    static double cache_noise();

    // ═══════════════════════════════════════════════════════════
    // Thread jitter — Multi-core scheduling noise
    // ═══════════════════════════════════════════════════════════
    static double thread_jitter();

    // ═══════════════════════════════════════════════════════════
    // Memory pattern noise — RAM access timing variations
    // ═══════════════════════════════════════════════════════════
    static double memory_pattern_noise();

    // ═══════════════════════════════════════════════════════════
    // Combined entropy from all hardware sources
    // ═══════════════════════════════════════════════════════════
    static double get_entropy() {
        double entropy = rdtsc_noise() * 1e-9;
        entropy = SafeMath::fmod_safe(entropy * PHI + cache_noise() * PSI);
        entropy = SafeMath::fmod_safe(entropy + thread_jitter() * 0.5);
        entropy = SafeMath::fmod_safe(entropy + memory_pattern_noise() * 0.3);
        return entropy;
    }
};

struct HardwareObfuscator {
    double entropy_state;
    HardwareCapabilities hw;

    void init();
    double obfuscate(double value);
    double generate_seed();
    double pq_obfuscate(double value);
    int safe_ring_dim() const { return hw.max_ring_dim; }
    int safe_ckks_depth() const { return hw.max_ckks_depth; }
    std::string report() { return hw.report(); }
};
