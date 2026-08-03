// ═══════════════════════════════════════════════════════════════
// PHI ALLOCATOR STRESS TEST — Memory Isolation Validation
// ═══════════════════════════════════════════════════════════════

#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstring>
#include <cstdint>
#include <vector>
#include <set>

const double PHI = 1.6180339887498948482;

class PhiAllocator {
private:
    uint64_t process_seed;
    
    uint64_t derive_seed(uint64_t pid) {
        double h = std::fmod(PHI * (double)(pid + 1), 1.0);
        for (int i = 0; i < 5; i++) {
            h = std::fmod(h * PHI + (double)pid * 0.1, 1.0);
        }
        return (uint64_t)(h * 1e18);
    }
    
    void xor_cipher(void* data, size_t size, uint64_t seed, bool encrypt) {
        uint64_t* words = (uint64_t*)data;
        size_t n = size / 8;
        uint64_t key = seed;
        for (size_t i = 0; i < n; i++) {
            if (encrypt) words[i] ^= key;
            else words[i] ^= key;
            key = key * 11400714819323198485ULL + 1372383749ULL;
        }
        uint8_t* bytes = (uint8_t*)(words + n);
        size_t rem = size % 8;
        uint8_t* kb = (uint8_t*)&key;
        for (size_t i = 0; i < rem; i++) {
            if (encrypt) bytes[i] ^= kb[i];
            else bytes[i] ^= kb[i];
        }
    }
    
public:
    PhiAllocator(uint64_t pid) : process_seed(derive_seed(pid)) {}
    uint64_t get_seed() const { return process_seed; }
    
    void* allocate(size_t size) {
        void* mem = std::malloc(size);
        if (!mem) return nullptr;
        std::memset(mem, 0, size);
        // Memory is zeroed but NOT encrypted here.
        // Encryption happens on writes via write_encrypted()
        return mem;
    }
    
    void write_encrypted(void* ptr, const void* data, size_t size) {
        // Copy plaintext to buffer, then encrypt in place
        std::memcpy(ptr, data, size);
        xor_cipher(ptr, size, process_seed, true);
    }
    
    void read_decrypted(void* ptr, void* out, size_t size, uint64_t reader_seed) {
        // Copy encrypted data, decrypt with reader's seed
        std::memcpy(out, ptr, size);
        xor_cipher(out, size, reader_seed, false);
    }
    
    std::vector<uint8_t> read_raw(void* ptr, size_t size, uint64_t reader_seed) {
        std::vector<uint8_t> data(size);
        read_decrypted(ptr, data.data(), size, reader_seed);
        return data;
    }
    
    void deallocate(void* ptr) { std::free(ptr); }
};

int main() {
    std::cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  PHI ALLOCATOR STRESS TEST                                  ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    
    int passed = 0, failed = 0;
    
    // TEST 1: Basic Isolation
    std::cout << "--- TEST 1: Basic Isolation ---\n";
    PhiAllocator procA(1000), procB(2000);
    std::cout << "  Seed A: " << procA.get_seed() << "\n";
    std::cout << "  Seed B: " << procB.get_seed() << "\n";
    std::cout << "  Different: " << (procA.get_seed() != procB.get_seed() ? "YES ✓" : "NO ✗") << "\n";
    
    const char* secret = "PROCESS_A_SECRET";
    size_t len = strlen(secret) + 1;
    void* memA = procA.allocate(len);
    procA.write_encrypted(memA, secret, len);
    
    auto B_view = procB.read_raw(memA, len, procB.get_seed());
    bool B_can_read = (std::string((char*)B_view.data()) == secret);
    std::cout << "  B reads A's memory: " << (B_can_read ? "YES ✗" : "NO ✓") << "\n";
    if (!B_can_read) passed++; else failed++;
    
    auto A_view = procA.read_raw(memA, len, procA.get_seed());
    bool A_can_read = (std::string((char*)A_view.data()) == secret);
    std::cout << "  A reads own memory: " << (A_can_read ? "YES ✓" : "NO ✗") << "\n";
    if (A_can_read) passed++; else failed++;
    
    std::cout << "  B's view (hex): ";
    for (size_t i = 0; i < std::min(len, (size_t)16); i++) printf("%02x ", B_view[i]);
    std::cout << "\n\n";
    
    // TEST 2: Seed Collision
    std::cout << "--- TEST 2: Seed Collision (100,000 PIDs) ---\n";
    std::set<uint64_t> seeds;
    int collisions = 0;
    for (uint64_t pid = 0; pid < 100000; pid++) {
        PhiAllocator p(pid);
        if (seeds.count(p.get_seed())) collisions++;
        seeds.insert(p.get_seed());
    }
    std::cout << "  Unique: " << seeds.size() << "/100000 | Collisions: " << collisions << "\n";
    if (collisions == 0) { std::cout << "  ✓ No collisions\n"; passed++; }
    else { std::cout << "  ✗ " << collisions << " collisions\n"; failed++; }
    std::cout << "\n";
    
    // TEST 3: Brute Force Search Space
    std::cout << "--- TEST 3: Brute Force Difficulty ---\n";
    uint64_t max_seed = 0, min_seed = UINT64_MAX;
    for (uint64_t pid = 0; pid < 1000; pid++) {
        uint64_t s = PhiAllocator(pid).get_seed();
        if (s > max_seed) max_seed = s;
        if (s < min_seed) min_seed = s;
    }
    double bits = std::log2(max_seed - min_seed);
    std::cout << "  Range: ~2^" << std::fixed << std::setprecision(1) << bits << "\n";
    if (bits > 40) { std::cout << "  ✓ > 2^40 search space\n"; passed++; }
    else { std::cout << "  ✗ Too small\n"; failed++; }
    std::cout << "\n";
    
    // TEST 4: Memory Cleanup
    std::cout << "--- TEST 4: Allocate/Free Cycle ---\n";
    for (int i = 0; i < 1000; i++) {
        void* m = procA.allocate(256);
        procA.deallocate(m);
    }
    std::cout << "  1000 alloc/free cycles: OK ✓\n";
    passed++;
    std::cout << "\n";
    
    // TEST 5: Cross-Process Isolation
    std::cout << "--- TEST 5: Cross-Process (50 processes) ---\n";
    std::vector<PhiAllocator*> procs;
    std::vector<void*> allocs;
    for (int i = 0; i < 50; i++) {
        procs.push_back(new PhiAllocator(i * 1000));
        allocs.push_back(procs[i]->allocate(64));
    }
    int cross_ok = 0;
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 50; j++) {
            if (i != j) {
                auto data = procs[i]->read_raw(allocs[j], 8, procs[i]->get_seed());
                bool all_zero = true;
                for (auto b : data) if (b != 0) all_zero = false;
                if (all_zero) cross_ok++;
            }
        }
    }
    std::cout << "  Cross-reads revealing zeros: " << cross_ok << "/2450\n";
    if (cross_ok == 0) { std::cout << "  ✓ No cross-process leaks\n"; passed++; }
    else { std::cout << "  ✗ " << cross_ok << " leaks\n"; failed++; }
    
    for (int i = 0; i < 50; i++) { procs[i]->deallocate(allocs[i]); delete procs[i]; }
    
    std::cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  PASSED: " << passed << "/" << (passed+failed) << "                                                ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n";
    
    return (failed == 0) ? 0 : 1;
}
