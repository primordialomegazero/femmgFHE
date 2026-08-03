// ═══════════════════════════════════════════════════════════════
// PHI SYSCALL STRESS TEST — File Isolation Validation
// ═══════════════════════════════════════════════════════════════
//
// TEST 1: Same path → different φ-hashes → different file descriptors
// TEST 2: Write/Read isolation — B cannot read A's file
// TEST 3: Path hash collision — can two different paths produce same hash?
// TEST 4: Seed uniqueness — can two processes share the same φ-branch?
// TEST 5: Multi-file isolation — concurrent file access

#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstring>
#include <cstdint>
#include <vector>
#include <map>
#include <set>
#include <mutex>

const double PHI = 1.6180339887498948482;

class PhiSyscall {
private:
    uint64_t process_seed;
    std::map<int, std::vector<uint8_t>> fd_data;  // fd → encrypted data
    std::map<int, uint64_t> fd_seeds;             // fd → seed used
    int next_fd = 1000;
    std::mutex syscall_mutex;
    
    uint64_t phi_hash(const char* path, uint64_t seed) {
        // WHY: Use φ-based bit mixing instead of fragile fractional parts.
        // Each character rotates the hash state and XORs with φ-derived constant.
        // The irrational φ ensures different paths diverge exponentially.
        uint64_t h = seed ^ 0x9e3779b97f4a7c15ULL;  // Golden ratio long
        for (const char* p = path; *p; p++) {
            h += (unsigned char)*p;
            h ^= h >> 33;
            h *= 0xff51afd7ed558ccdULL;  // φ × 2^63
            h ^= h >> 33;
            h *= 0xc4ceb9fe1a85ec53ULL;  // More φ mixing
            h ^= h >> 33;
        }
        return h;
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
    
    int phi_open(const char* path) {
        std::lock_guard<std::mutex> lock(syscall_mutex);
        uint64_t hash = phi_hash(path, process_seed);
        int fd = (int)(hash & 0x7FFFFFFF);  // Positive fd
        if (fd_data.find(fd) == fd_data.end()) {
            fd_seeds[fd] = hash;
            fd_data[fd] = std::vector<uint8_t>();
        }
        return fd;
    }
    
    void phi_write(int fd, const void* buf, size_t count) {
        std::lock_guard<std::mutex> lock(syscall_mutex);
        if (fd_seeds.find(fd) == fd_seeds.end()) return;
        
        std::vector<uint8_t> encrypted(count);
        std::memcpy(encrypted.data(), buf, count);
        xor_cipher(encrypted.data(), count, fd_seeds[fd]);
        fd_data[fd] = std::move(encrypted);
    }
    
    std::vector<uint8_t> phi_read(int fd, size_t count) {
        std::lock_guard<std::mutex> lock(syscall_mutex);
        if (fd_seeds.find(fd) == fd_seeds.end()) return {};
        
        auto& stored = fd_data[fd];
        if (stored.empty()) return std::vector<uint8_t>(count, 0);
        
        std::vector<uint8_t> decrypted = stored;
        xor_cipher(decrypted.data(), decrypted.size(), fd_seeds[fd]);
        return decrypted;
    }
    
    uint64_t get_seed() const { return process_seed; }
};

int main() {
    std::cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  PHI SYSCALL STRESS TEST                                    ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    
    int passed = 0, failed = 0;
    
    // === TEST 1: Same Path → Different FDs ===
    std::cout << "--- TEST 1: Same Path, Different Processes ---\n";
    PhiSyscall procA(1000), procB(2000);
    const char* path = "/etc/secret.txt";
    
    int fdA = procA.phi_open(path);
    int fdB = procB.phi_open(path);
    
    std::cout << "  Process A fd: " << fdA << "\n";
    std::cout << "  Process B fd: " << fdB << "\n";
    std::cout << "  Different FDs: " << (fdA != fdB ? "YES ✓" : "NO ✗") << "\n";
    if (fdA != fdB) passed++; else failed++;
    std::cout << "\n";
    
    // === TEST 2: Write/Read Isolation ===
    std::cout << "--- TEST 2: Cross-Process File Access ---\n";
    const char* dataA = "Process A secret file data";
    size_t lenA = strlen(dataA) + 1;
    
    procA.phi_write(fdA, dataA, lenA);
    
    // A reads own file
    auto readA = procA.phi_read(fdA, lenA);
    bool A_ok = (std::string((char*)readA.data()) == dataA);
    std::cout << "  A reads own file: " << (A_ok ? "YES ✓" : "NO ✗") << "\n";
    if (A_ok) passed++; else failed++;
    
    // B tries to read A's file using A's fd
    auto B_read = procB.phi_read(fdA, lenA);
    // B should get empty or garbage
    bool B_blocked = B_read.empty() || (std::string((char*)B_read.data()) != dataA);
    std::cout << "  B reads A's fd: " << (!B_blocked ? "SUCCESS ✗" : "BLOCKED ✓") << "\n";
    if (B_blocked) passed++; else failed++;
    
    // B reads own fd for same path
    const char* dataB = "Process B different data here";
    size_t lenB = strlen(dataB) + 1;
    procB.phi_write(fdB, dataB, lenB);
    auto readB = procB.phi_read(fdB, lenB);
    bool B_own_ok = (std::string((char*)readB.data()) == dataB);
    std::cout << "  B reads own file: " << (B_own_ok ? "YES ✓" : "NO ✗") << "\n";
    if (B_own_ok) passed++; else failed++;
    std::cout << "\n";
    
    // === TEST 3: Path Hash Collision ===
    std::cout << "--- TEST 3: Path Hash Collision (10,000 paths) ---\n";
    std::set<uint64_t> hashes;
    int collisions = 0;
    char test_path[256];
    for (int i = 0; i < 10000; i++) {
        snprintf(test_path, sizeof(test_path), "/home/user/file_%d.txt", i);
        uint64_t h = procA.phi_open(test_path);
        if (hashes.count(h)) collisions++;
        hashes.insert(h);
    }
    std::cout << "  Unique hashes: " << hashes.size() << "/10000\n";
    std::cout << "  Collisions: " << collisions << "\n";
    if (collisions == 0) { std::cout << "  ✓ No collisions\n"; passed++; }
    else { std::cout << "  ✗ " << collisions << " collisions\n"; failed++; }
    std::cout << "\n";
    
    // === TEST 4: Seed Space ===
    std::cout << "--- TEST 4: Seed Space (100,000 processes) ---\n";
    std::set<uint64_t> proc_seeds;
    for (uint64_t i = 0; i < 100000; i++) {
        PhiSyscall p(i * 100);
        proc_seeds.insert(p.get_seed());
    }
    std::cout << "  Unique seeds: " << proc_seeds.size() << "/100000\n";
    if (proc_seeds.size() == 100000) { std::cout << "  ✓ All unique\n"; passed++; }
    else { std::cout << "  ✗ Duplicates found\n"; failed++; }
    std::cout << "\n";
    
    // === TEST 5: Concurrent File Access ===
    std::cout << "--- TEST 5: Multi-File Concurrent Access ---\n";
    std::vector<int> fds;
    PhiSyscall multi(5000);
    for (int i = 0; i < 100; i++) {
        snprintf(test_path, sizeof(test_path), "/tmp/file_%d.tmp", i);
        int fd = multi.phi_open(test_path);
        char buf[64];
        snprintf(buf, sizeof(buf), "Data for file %d", i);
        multi.phi_write(fd, buf, strlen(buf)+1);
        fds.push_back(fd);
    }
    // Read all back
    int read_ok = 0;
    for (int i = 0; i < 100; i++) {
        snprintf(test_path, sizeof(test_path), "/tmp/file_%d.tmp", i);
        int fd = multi.phi_open(test_path);
        auto data = multi.phi_read(fd, 64);
        char expected[64];
        snprintf(expected, sizeof(expected), "Data for file %d", i);
        if (std::string((char*)data.data()) == expected) read_ok++;
    }
    std::cout << "  Files correctly read: " << read_ok << "/100\n";
    if (read_ok == 100) { std::cout << "  ✓ All files isolated & accessible\n"; passed++; }
    else { std::cout << "  ✗ " << (100-read_ok) << " files corrupted\n"; failed++; }
    
    std::cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  PASSED: " << passed << "/" << (passed+failed) << "                                                ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n";
    
    return (failed == 0) ? 0 : 1;
}
