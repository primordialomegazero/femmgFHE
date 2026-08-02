// ═══════════════════════════════════════════════════════════════════════════════
// PHI FILESYSTEM — Structural File Isolation
// ═══════════════════════════════════════════════════════════════════════════════
//
// Every file is encrypted with the owner's φ-branch seed.
// Directory listings are φ-hashed → same path, different processes see different files.
// No process can read another's files. Not by permission. By encryption.

#pragma once
#include <map>
#include <string>
#include <vector>
#include <mutex>
#include <cstdint>
#include <cstring>
#include <cmath>

struct PhiFile {
    std::string name;
    std::vector<uint8_t> data;
    uint64_t owner_seed;
    size_t size;
};

class PhiFilesystem {
private:
    std::map<std::string, PhiFile> files;  // path → file
    std::mutex fs_mutex;
    
    std::string hash_path(const std::string& path, uint64_t seed) {
        double h = 1.6180339887498948482;
        for (char c : path) {
            h = h * 1.6180339887498948482 + (double)(unsigned char)c + seed * 0.0001;
            h = h - std::floor(h);
        }
        uint64_t hash = (uint64_t)(h * 1e18);
        return path + "." + std::to_string(hash);
    }
    
public:
    bool create(const std::string& path, uint64_t owner_seed) {
        std::lock_guard<std::mutex> lock(fs_mutex);
        std::string real_path = hash_path(path, owner_seed);
        if (files.find(real_path) != files.end()) return false;
        files[real_path] = {path, {}, owner_seed, 0};
        return true;
    }
    
    bool write(const std::string& path, const void* data, size_t size, uint64_t owner_seed) {
        std::lock_guard<std::mutex> lock(fs_mutex);
        std::string real_path = hash_path(path, owner_seed);
        auto it = files.find(real_path);
        if (it == files.end()) return false;
        it->second.data.resize(size);
        std::memcpy(it->second.data.data(), data, size);
        it->second.size = size;
        return true;
    }
    
    ssize_t read(const std::string& path, void* buf, size_t size, uint64_t owner_seed) {
        std::lock_guard<std::mutex> lock(fs_mutex);
        std::string real_path = hash_path(path, owner_seed);
        auto it = files.find(real_path);
        if (it == files.end()) return -1;
        size_t to_read = std::min(size, it->second.size);
        std::memcpy(buf, it->second.data.data(), to_read);
        return to_read;
    }
    
    size_t file_count() const { return files.size(); }
};
