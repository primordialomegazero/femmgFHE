#include <iostream>
#include <iomanip>
#include <cstring>
#include <cstdint>
#include <cmath>
#include <vector>

const double PHI = 1.6180339887498948482;

uint64_t derive_seed(uint64_t pid) {
    double h = std::fmod(PHI * (double)(pid + 1), 1.0);
    for (int i = 0; i < 5; i++) {
        h = std::fmod(h * PHI + (double)pid * 0.1, 1.0);
    }
    return (uint64_t)(h * 1e18);
}

// EXACT copy of the original xor_cipher
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

// EXACT copy of the original read_raw
std::vector<uint8_t> read_raw(void* ptr, size_t size, uint64_t reader_seed) {
    std::vector<uint8_t> data(size);
    std::memcpy(data.data(), ptr, size);
    // Decrypt with reader's seed
    uint64_t* words = (uint64_t*)data.data();
    size_t n = size / 8;
    uint64_t key = reader_seed;
    for (size_t i = 0; i < n; i++) {
        words[i] ^= key;
        key = key * 11400714819323198485ULL + 1372383749ULL;
    }
    uint8_t* bytes = (uint8_t*)(words + n);
    size_t rem = size % 8;
    uint8_t* kb = (uint8_t*)&key;
    for (size_t i = 0; i < rem; i++) bytes[i] ^= kb[i];
    return data;
}

int main() {
    uint64_t seedA = derive_seed(1000);
    
    std::cout << "Seed A: " << seedA << "\n\n";
    
    const char* secret = "PROCESS_A_SECRET";
    size_t len = strlen(secret) + 1;
    
    // STEP 1: Allocate and encrypt (what allocate() does)
    void* mem = std::malloc(len);
    std::memset(mem, 0, len);
    std::memcpy(mem, secret, len);
    
    std::cout << "Original: '" << (char*)mem << "'\n";
    
    // Print bytes before encryption
    std::cout << "Before encrypt: ";
    for (size_t i = 0; i < len; i++) printf("%02x ", ((uint8_t*)mem)[i]);
    std::cout << "\n";
    
    xor_encrypt(mem, len, seedA);
    
    std::cout << "After encrypt:  ";
    for (size_t i = 0; i < len; i++) printf("%02x ", ((uint8_t*)mem)[i]);
    std::cout << "\n";
    
    // STEP 2: Read back (what read_raw does)
    auto decrypted = read_raw(mem, len, seedA);
    
    std::cout << "After decrypt:  ";
    for (size_t i = 0; i < len; i++) printf("%02x ", decrypted[i]);
    std::cout << "\n";
    
    std::cout << "Decrypted: '" << (char*)decrypted.data() << "'\n";
    std::cout << "Match: " << (std::string((char*)decrypted.data()) == secret ? "YES ✓" : "NO ✗") << "\n";
    
    // Check: is the key stream reproducible?
    std::cout << "\n--- Key stream check ---\n";
    uint64_t key1 = seedA;
    uint64_t key2 = seedA;
    for (size_t i = 0; i < 3; i++) {
        std::cout << "  Round " << i << ": key1=" << key1 << " key2=" << key2;
        std::cout << " match=" << (key1 == key2 ? "YES" : "NO") << "\n";
        key1 = key1 * 11400714819323198485ULL + 1372383749ULL;
        key2 = key2 * 11400714819323198485ULL + 1372383749ULL;
    }
    
    std::free(mem);
    return 0;
}
