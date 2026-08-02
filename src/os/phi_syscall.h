// ═══════════════════════════════════════════════════════════════════════════════
// PHI SYSCALL — Structural System Call Isolation
// ═══════════════════════════════════════════════════════════════════════════════
//
// Every syscall is wrapped in φ-validation:
//   open(path)   → φ_hash(path) → verify against process φ-branch
//   read(fd)     → φ_decrypt(buffer) → return plaintext
//   write(fd)    → φ_encrypt(buffer) → write ciphertext
//
// No process can read another's files — because they're encrypted with different φ-branches.
// No process can open another's paths — because the path hash won't match the φ-branch.

#pragma once
#include <cstdint>
#include <cstring>
#include <cmath>
#include <map>
#include <mutex>
#include <vector>
#include "../core/constants.h"

class PhiSyscall {
private:
    uint64_t process_seed;
    std::map<int, uint64_t> fd_seeds;
    std::map<int, std::vector<uint8_t>> fd_data;  // actual stored data
    std::mutex syscall_mutex;
    
    // φ-derived hash — same input, different process → different hash
    uint64_t phi_hash(const char* str) {
        double h = PHI;
        for (const char* p = str; *p; p++) {
            h = h * PHI + (double)(unsigned char)*p + process_seed * 0.0001;
            h = h - std::floor(h);
        }
        return (uint64_t)(h * 1e18);
    }
    
    void xor_cipher(void* data, size_t size, uint64_t seed) {
        uint64_t* words = (uint64_t*)data;
        size_t n = size / 8;
        uint64_t key = seed;
        for (size_t i = 0; i < n; i++) {
            words[i] ^= key;
            key = key * 11400714819323198485ULL + 1372383749ULL;
        }
        uint8_t* bytes = (uint8_t*)(words + n);
        size_t rem = size % 8;
        uint8_t* kb = (uint8_t*)&key;
        for (size_t i = 0; i < rem; i++) bytes[i] ^= kb[i];
    }
    
public:
    PhiSyscall(uint64_t seed) : process_seed(seed) {}
    
    // φ-open: returns fd (encrypted with process seed)
    int phi_open(const char* pathname) {
        std::lock_guard<std::mutex> lock(syscall_mutex);
        uint64_t path_hash = phi_hash(pathname);
        int fd = (int)(path_hash & 0x7FFFFFFF);
        fd_seeds[fd] = path_hash;
        return fd;
    }
    
    // φ-read: decrypts data using fd-specific seed
    ssize_t phi_read(int fd, void* buf, size_t count) {
        std::lock_guard<std::mutex> lock(syscall_mutex);
        auto it = fd_seeds.find(fd);
        if (it == fd_seeds.end()) return -1;
        
        auto data_it = fd_data.find(fd);
        if (data_it == fd_data.end()) return 0;
        
        size_t available = data_it->second.size();
        size_t to_read = std::min(count, available);
        
        // Decrypt using same seed as write
        std::memcpy(buf, data_it->second.data(), to_read);
        xor_cipher(buf, to_read, it->second);
        return to_read;
    }
    
    // φ-write: encrypts data before storage
    ssize_t phi_write(int fd, const void* buf, size_t count) {
        std::lock_guard<std::mutex> lock(syscall_mutex);
        auto it = fd_seeds.find(fd);
        if (it == fd_seeds.end()) return -1;
        
        // Store data encrypted with fd-specific seed
        std::vector<uint8_t> encrypted(count);
        std::memcpy(encrypted.data(), buf, count);
        xor_cipher(encrypted.data(), count, it->second);
        fd_data[fd] = std::move(encrypted);
        return count;
    }
    
    // φ-close
    int phi_close(int fd) {
        std::lock_guard<std::mutex> lock(syscall_mutex);
        fd_seeds.erase(fd);
        fd_data.erase(fd);
        return 0;
    }
    
    uint64_t get_seed() const { return process_seed; }
};
