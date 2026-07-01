/*
 * BLACKHOLE + GOLDEN CHAOS — SIMPLIFIED
 */

#pragma once
#include "golden_chaos.h"
#include <vector>
#include <cstdint>
#include <cmath>
#include <iostream>

namespace blackhole_simple {

using namespace golden_chaos;

constexpr size_t GOLDEN_LAYERS = 14;
constexpr uint64_t PHI_HASH_MAGIC = 0x9E3779B97F4A7C15ULL;

class SimpleBlackhole {
private:
    GoldenChaosEngine chaos_;
    uint64_t counter_ = 0;
    
public:
    // ═══ ENCRYPT: Golden Chaos only ═══
    std::vector<uint8_t> encrypt(const std::vector<uint8_t>& data, uint64_t key) {
        counter_++;
        
        std::array<double, GOLDEN_LAYERS> chaos_history{};
        std::vector<uint8_t> result;
        result.reserve(data.size() + GOLDEN_LAYERS * 8);
        
        // Apply golden chaos to each byte
        for (auto byte : data) {
            auto [chaos_val, history] = chaos_.observe(static_cast<double>(byte), counter_);
            chaos_history = history;
            result.push_back(static_cast<uint8_t>(std::fmod(std::abs(chaos_val), 256.0)));
        }
        
        // Append chaos history (for decryption)
        for (double val : chaos_history) {
            uint64_t bits = *reinterpret_cast<uint64_t*>(&val);
            for (int b = 0; b < 8; b++) {
                result.push_back(static_cast<uint8_t>((bits >> (b * 8)) & 0xFF));
            }
        }
        
        // XOR with key
        for (size_t i = 0; i < result.size(); i++) {
            result[i] ^= static_cast<uint8_t>((key >> ((i % 8) * 8)) & 0xFF);
        }
        
        return result;
    }
    
    // ═══ DECRYPT: Reverse golden chaos ═══
    std::vector<uint8_t> decrypt(const std::vector<uint8_t>& ciphertext, uint64_t key) {
        if (ciphertext.size() < GOLDEN_LAYERS * 8) {
            return {};
        }
        
        // XOR with key
        std::vector<uint8_t> decrypted = ciphertext;
        for (size_t i = 0; i < decrypted.size(); i++) {
            decrypted[i] ^= static_cast<uint8_t>((key >> ((i % 8) * 8)) & 0xFF);
        }
        
        // Extract chaos history (last GOLDEN_LAYERS * 8 bytes)
        size_t data_size = decrypted.size() - GOLDEN_LAYERS * 8;
        std::array<double, GOLDEN_LAYERS> chaos_history{};
        size_t offset = data_size;
        for (int i = 0; i < GOLDEN_LAYERS; i++) {
            uint64_t bits = 0;
            for (int b = 0; b < 8; b++) {
                bits |= static_cast<uint64_t>(decrypted[offset++]) << (b * 8);
            }
            chaos_history[i] = *reinterpret_cast<double*>(&bits);
        }
        
        // Decrypt data using golden chaos
        std::vector<uint8_t> result;
        result.reserve(data_size);
        for (size_t i = 0; i < data_size; i++) {
            double val = static_cast<double>(decrypted[i]);
            double unobserve_val = chaos_.unobserve(val, chaos_history, 0);
            result.push_back(static_cast<uint8_t>(std::fmod(std::abs(unobserve_val), 256.0)));
        }
        
        return result;
    }
};

inline bool test() {
    SimpleBlackhole bh;
    uint64_t key = 0xDEADBEEF12345678ULL;
    
    std::vector<uint8_t> plaintext = {42, 43, 44, 45, 46, 47, 48, 49};
    
    std::cout << "BLACKHOLE + GOLDEN CHAOS TEST (SIMPLIFIED)" << std::endl;
    std::cout << "Plaintext: ";
    for (auto b : plaintext) std::cout << (int)b << " ";
    std::cout << std::endl;
    
    auto encrypted = bh.encrypt(plaintext, key);
    std::cout << "Encrypted size: " << encrypted.size() << " bytes" << std::endl;
    
    auto decrypted = bh.decrypt(encrypted, key);
    std::cout << "Decrypted: ";
    for (auto b : decrypted) std::cout << (int)b << " ";
    std::cout << std::endl;
    
    bool ok = (decrypted == plaintext);
    std::cout << "Status: " << (ok ? "✅ PASS" : "❌ FAIL") << std::endl;
    
    return ok;
}

} // namespace blackhole_simple
