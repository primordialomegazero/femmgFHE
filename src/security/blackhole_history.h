/*
 * BLACKHOLE + GOLDEN CHAOS — FIXED v2
 *
 * Bug: uint8_t truncation loses fractional part.
 * Fix: Store exact chaos_val as double (8 bytes).
 */

#pragma once
#include "golden_chaos.h"
#include <vector>
#include <cstdint>
#include <cmath>
#include <cstring>
#include <iostream>

namespace blackhole_fixed {

using namespace golden_chaos;

constexpr size_t GOLDEN_LAYERS = 14;

class FixedBlackhole {
private:
    GoldenChaosEngine chaos_;
    uint64_t counter_ = 0;

public:
    // ═══ ENCRYPT: Store chaos_val (8 bytes) + history (14×8 bytes) ═══
    std::vector<uint8_t> encrypt(const std::vector<uint8_t>& data, uint64_t key) {
        counter_++;
        
        std::vector<uint8_t> result;
        // Format: [num_bytes:8][chaos_val0:8][history0:14*8][chaos_val1:8][history1:14*8]...
        uint64_t num_bytes = data.size();
        for (int i = 0; i < 8; i++) {
            result.push_back((num_bytes >> (i * 8)) & 0xFF);
        }
        
        for (auto byte : data) {
            // Encrypt ONE byte — get exact chaos_val (double)
            auto [chaos_val, history] = chaos_.observe(
                static_cast<double>(byte), 
                counter_ + byte
            );
            
            // Store EXACT chaos_val (8 bytes) — no truncation!
            uint64_t chaos_bits;
            std::memcpy(&chaos_bits, &chaos_val, sizeof(double));
            for (int i = 0; i < 8; i++) {
                result.push_back((chaos_bits >> (i * 8)) & 0xFF);
            }
            
            // Store chaos history (14 × 8 bytes)
            for (double val : history) {
                uint64_t bits;
                std::memcpy(&bits, &val, sizeof(double));
                for (int i = 0; i < 8; i++) {
                    result.push_back((bits >> (i * 8)) & 0xFF);
                }
            }
        }
        
        return result;
    }
    
    // ═══ DECRYPT: Use exact chaos_val + history ═══
    std::vector<uint8_t> decrypt(const std::vector<uint8_t>& encrypted, uint64_t key) {
        if (encrypted.size() < 8) return {};
        
        // Read number of bytes
        uint64_t num_bytes = 0;
        for (int i = 0; i < 8; i++) {
            num_bytes |= static_cast<uint64_t>(encrypted[i]) << (i * 8);
        }
        
        std::vector<uint8_t> result;
        size_t pos = 8;
        
        for (uint64_t b = 0; b < num_bytes && pos < encrypted.size(); b++) {
            // Read EXACT chaos_val (8 bytes)
            uint64_t chaos_bits = 0;
            for (int i = 0; i < 8; i++) {
                chaos_bits |= static_cast<uint64_t>(encrypted[pos++]) << (i * 8);
            }
            double chaos_val;
            std::memcpy(&chaos_val, &chaos_bits, sizeof(double));
            
            // Read chaos history (14 × 8 bytes)
            std::array<double, GOLDEN_LAYERS> history{};
            for (int i = 0; i < GOLDEN_LAYERS && pos + 8 <= encrypted.size(); i++) {
                uint64_t bits = 0;
                for (int j = 0; j < 8; j++) {
                    bits |= static_cast<uint64_t>(encrypted[pos++]) << (j * 8);
                }
                std::memcpy(&history[i], &bits, sizeof(double));
            }
            
            // Unobserve with EXACT chaos_val — no truncation!
            double decrypted_val = chaos_.unobserve(chaos_val, history, 0);
            
            result.push_back(static_cast<uint8_t>(
                std::round(decrypted_val)  // Should be exact integer
            ));
        }
        
        return result;
    }
};

} // namespace blackhole_fixed
