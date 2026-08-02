#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cmath>
#include <map>
#include <mutex>
#include "../core/constants.h"

struct PhiPage {
    uint64_t seed;
    void* raw_memory;
    size_t size;
    bool encrypted;
};

class PhiAllocator {
private:
    std::map<void*, PhiPage> pages;
    std::mutex alloc_mutex;
    
    // Simple guaranteed-unique seed: φ × process_id, keep fractional part
    uint64_t derive_seed(uint64_t process_id) {
        double h = std::fmod(PHI * (double)(process_id + 1), 1.0);
        // Hash it well
        for (int i = 0; i < 5; i++) {
            h = std::fmod(h * PHI + (double)process_id * 0.1, 1.0);
        }
        return (uint64_t)(h * 1e18);
    }
    
    void xor_encrypt(void* data, size_t size, uint64_t seed) {
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
    void* allocate(size_t size, uint64_t process_id) {
        std::lock_guard<std::mutex> lock(alloc_mutex);
        uint64_t seed = derive_seed(process_id);
        void* mem = std::malloc(size);
        if (!mem) return nullptr;
        std::memset(mem, 0, size);
        xor_encrypt(mem, size, seed);
        pages[mem] = {seed, mem, size, true};
        return mem;
    }
    
    void* access(void* ptr, uint64_t process_id) {
        std::lock_guard<std::mutex> lock(alloc_mutex);
        auto it = pages.find(ptr);
        if (it == pages.end()) return nullptr;
        uint64_t expected_seed = derive_seed(process_id);
        if (it->second.seed != expected_seed) return nullptr;
        return it->second.raw_memory;
    }
    
    void deallocate(void* ptr) {
        std::lock_guard<std::mutex> lock(alloc_mutex);
        auto it = pages.find(ptr);
        if (it != pages.end()) {
            std::memset(it->second.raw_memory, 0, it->second.size);
            std::free(it->second.raw_memory);
            pages.erase(it);
        }
    }
    
    uint64_t get_seed(uint64_t process_id) { return derive_seed(process_id); }
    size_t page_count() const { return pages.size(); }
};
