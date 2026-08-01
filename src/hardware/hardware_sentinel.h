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
#include <sys/sysinfo.h>

// ═══════════════════════════════════════════════════════════════════════════════
// HARDWARE SENTINEL — Auto-Detect & Post-Quantum Hardware Obfuscation
// ═══════════════════════════════════════════════════════════════════════════════

struct HardwareCapabilities {
    int max_ring_dim;
    int max_ckks_depth;
    int cpu_cores;
    size_t total_ram_gb;
    bool has_avx2;
    bool has_avx512;
    bool has_aes_ni;
    std::string cpu_brand;
    std::string os_name;
    
    static HardwareCapabilities detect() {
        HardwareCapabilities hw;
        
        // Detect RAM
        long pages = sysconf(_SC_PHYS_PAGES);
        long page_size = sysconf(_SC_PAGE_SIZE);
        size_t total_ram = pages * page_size;
        hw.total_ram_gb = total_ram / (1024ULL * 1024 * 1024);
        
        // Map RAM to max RingDim
        if (hw.total_ram_gb < 4)       hw.max_ring_dim = 2048;
        else if (hw.total_ram_gb < 8)  hw.max_ring_dim = 4096;
        else if (hw.total_ram_gb < 16) hw.max_ring_dim = 8192;
        else if (hw.total_ram_gb < 32) hw.max_ring_dim = 16384;
        else if (hw.total_ram_gb < 128) hw.max_ring_dim = 32768;
        else                            hw.max_ring_dim = 65536;
        
        // Map RingDim to CKKS depth
        hw.max_ckks_depth = (hw.max_ring_dim >= 32768) ? 300 :
                           (hw.max_ring_dim >= 16384) ? 200 :
                           (hw.max_ring_dim >= 8192)  ? 120 : 60;
        
        // CPU cores
        hw.cpu_cores = std::thread::hardware_concurrency();
        
        // CPU features
        #ifdef __GNUC__
            hw.has_avx2 = __builtin_cpu_supports("avx2");
            hw.has_avx512 = __builtin_cpu_supports("avx512f");
            hw.has_aes_ni = __builtin_cpu_supports("aes");
        #else
            hw.has_avx2 = false;
            hw.has_avx512 = false;
            hw.has_aes_ni = false;
        #endif
        
        // OS
        #ifdef __linux__
            hw.os_name = "Linux";
        #elif defined(__APPLE__)
            hw.os_name = "macOS";
        #elif defined(_WIN32)
            hw.os_name = "Windows";
        #else
            hw.os_name = "Unknown";
        #endif
        
        // CPU brand
        hw.cpu_brand = get_cpu_brand();
        
        return hw;
    }
    
    static std::string get_cpu_brand() {
        std::string brand = "Unknown";
        #ifdef __linux__
            FILE* fp = popen("cat /proc/cpuinfo | grep 'model name' | head -1 | cut -d: -f2", "r");
            if (fp) {
                char buf[256];
                if (fgets(buf, sizeof(buf), fp)) {
                    brand = buf;
                    brand.erase(0, brand.find_first_not_of(" \t\n"));
                    brand.erase(brand.find_last_not_of(" \t\n") + 1);
                }
                pclose(fp);
            }
        #endif
        return brand.empty() ? "Unknown" : brand;
    }
    
    std::string report() {
        std::stringstream ss;
        ss << "\n";
        ss << "╔══════════════════════════════════════════════════════╗\n";
        ss << "║  HARDWARE CAPABILITIES                              ║\n";
        ss << "╠══════════════════════════════════════════════════════╣\n";
        ss << "║  CPU:      " << std::setw(40) << std::left << cpu_brand.substr(0,40) << " ║\n";
        ss << "║  Cores:    " << std::setw(40) << std::to_string(cpu_cores) + " threads" << " ║\n";
        ss << "║  RAM:      " << std::setw(40) << std::to_string(total_ram_gb) + " GB" << " ║\n";
        ss << "║  OS:       " << std::setw(40) << os_name << " ║\n";
        ss << "║  AVX2:     " << std::setw(40) << (has_avx2 ? "YES" : "NO") << " ║\n";
        ss << "║  AVX-512:  " << std::setw(40) << (has_avx512 ? "YES" : "NO") << " ║\n";
        ss << "║  AES-NI:   " << std::setw(40) << (has_aes_ni ? "YES" : "NO") << " ║\n";
        ss << "╠══════════════════════════════════════════════════════╣\n";
        ss << "║  Max RingDim:  " << std::setw(4) << max_ring_dim << "                                ║\n";
        ss << "║  Max Depth:    " << std::setw(4) << max_ckks_depth << "                                ║\n";
        ss << "╚══════════════════════════════════════════════════════╝\n";
        return ss.str();
    }
};

// ═══════════════════════════════════════════════════════════════
// HARDWARE ENTROPY SOURCES
// ═══════════════════════════════════════════════════════════════
struct HardwareEntropy {
    static uint64_t rdtsc_noise() {
        uint32_t lo, hi;
        __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
        return ((uint64_t)hi << 32) | lo;
    }
    
    static double cache_noise() {
        const int BUF_SIZE = 4096;
        volatile char buf[BUF_SIZE];
        std::memset((void*)buf, 0, BUF_SIZE);
        
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < BUF_SIZE; i += 64) {
            buf[i] = (char)(buf[i] + 1);
        }
        auto end = std::chrono::high_resolution_clock::now();
        
        double nanos = std::chrono::duration<double, std::nano>(end - start).count();
        return SafeMath::fmod_safe(nanos * 1e-9 * PHI);
    }
    
    static double thread_jitter() {
        std::atomic<double> result{0.0};
        std::atomic<bool> ready{false};
        
        std::thread t([&]() {
            ready.store(true);
            auto start = std::chrono::high_resolution_clock::now();
            while (result.load() == 0.0) {
                volatile int x = 0;
                x++;
            }
            auto end = std::chrono::high_resolution_clock::now();
            double nanos = std::chrono::duration<double, std::nano>(end - start).count();
            result.store(nanos);
        });
        
        while (!ready.load()) {}
        
        uint64_t noise = rdtsc_noise();
        for (volatile int i = 0; i < (noise % 1000); i++) {}
        
        result.store(1.0);
        t.join();
        
        return SafeMath::fmod_safe(result.load() * 1e-9 * PSI);
    }
    
    static double memory_pattern_noise() {
        const int SIZE = 1024 * 1024;
        volatile char* buf = new volatile char[SIZE];
        
        uint64_t seed = rdtsc_noise();
        for (int i = 0; i < SIZE; i++) {
            seed = seed * 1103515245 + 12345;
            buf[i] = (char)(seed & 0xFF);
        }
        
        auto start = std::chrono::high_resolution_clock::now();
        double sum = 0;
        for (int i = 0; i < SIZE; i += (seed % 64 + 1)) {
            sum += buf[i];
        }
        auto end = std::chrono::high_resolution_clock::now();
        
        delete[] buf;
        
        double nanos = std::chrono::duration<double, std::nano>(end - start).count();
        return SafeMath::fmod_safe(nanos * 1e-9 * PHI + sum * 1e-6);
    }
    
    static double get_entropy() {
        double entropy = rdtsc_noise() * 1e-9;
        entropy = SafeMath::fmod_safe(entropy * PHI + cache_noise() * PSI);
        entropy = SafeMath::fmod_safe(entropy + thread_jitter() * 0.5);
        entropy = SafeMath::fmod_safe(entropy + memory_pattern_noise() * 0.3);
        return entropy;
    }
};

// ═══════════════════════════════════════════════════════════════
// HARDWARE POST-QUANTUM OBFUSCATOR
// ═══════════════════════════════════════════════════════════════
struct HardwareObfuscator {
    double entropy_state;
    HardwareCapabilities hw;
    
    void init() {
        hw = HardwareCapabilities::detect();
        entropy_state = HardwareEntropy::get_entropy();
        Logger::info("HardwareObfuscator: " + std::to_string(hw.cpu_cores) + 
                    " cores, " + std::to_string(hw.total_ram_gb) + "GB, " +
                    hw.os_name);
    }
    
    double obfuscate(double value) {
        double hw_noise = HardwareEntropy::get_entropy();
        entropy_state = 3.99 * entropy_state * (1.0 - entropy_state);
        double mixed = SafeMath::fmod_safe(value * PHI + hw_noise * PSI + entropy_state);
        mixed = SafeMath::fmod_safe(mixed + HardwareEntropy::cache_noise() * 0.001);
        return mixed;
    }
    
    double generate_seed() {
        double seed = HardwareEntropy::rdtsc_noise() * 1e-9;
        seed = SafeMath::fmod_safe(seed * PHI);
        seed = SafeMath::fmod_safe(seed + HardwareEntropy::thread_jitter());
        seed = SafeMath::fmod_safe(seed + HardwareEntropy::memory_pattern_noise());
        return seed;
    }
    
    double pq_obfuscate(double value) {
        double hw_fingerprint = SafeMath::fmod_safe(
            hw.cpu_cores * 0.1 + 
            hw.total_ram_gb * 0.01 +
            (hw.has_avx512 ? PHI : 1.0) * 0.05
        );
        value = SafeMath::fmod_safe(value * hw_fingerprint + entropy_state);
        return obfuscate(value);
    }
    
    int safe_ring_dim() const { return hw.max_ring_dim; }
    int safe_ckks_depth() const { return hw.max_ckks_depth; }
    std::string report() { return hw.report(); }
};
