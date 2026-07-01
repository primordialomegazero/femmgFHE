/*
 * FEmmg-FHE v22.1 — Fibonacci Reversal Chaos
 * 
 * Third chaos engine: Fibonacci-based bit reversal.
 * 
 * Instead of adding chaos forward, this engine:
 * - Extracts digits from the value
 * - Reorders them using Fibonacci sequence indices
 * - Reverses the φ-spiral direction
 * 
 * "Forward is chaos. Backward is Fibonacci. Both are φ."
 */

#pragma once
#include <cmath>
#include <cstdint>
#include <array>

namespace fibonacci_reversal {

constexpr double PHI = 1.6180339887498948482;
constexpr double PHI_INV = 0.6180339887498948482;
constexpr int LAYERS = 14;
constexpr int DIGITS = 12;  // Decimal digits to scramble

// Fibonacci sequence (first 14)
constexpr uint64_t FIB[14] = {1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610};

class FibonacciReversalEngine {
private:
    uint64_t global_nonce_{0x9E3779B97F4A7C15ULL};
    uint64_t operation_counter_{0};
    
    // ═══ FIBONACCI SCRAMBLE ═══
    // Extracts digits, reorders by Fibonacci indices, reverses
    double fib_scramble(double value, uint64_t nonce, bool forward) {
        // Extract digits as integer parts
        int64_t int_part = static_cast<int64_t>(std::abs(value) * 1e6);
        double frac_part = value - static_cast<int64_t>(value);
        
        // Break into "digits" (6-digit chunks)
        std::array<int64_t, DIGITS> chunks{};
        int64_t temp = int_part;
        for (int i = 0; i < DIGITS; i++) {
            chunks[i] = temp % 1000000;
            temp /= 1000000;
        }
        
        // Fibonacci reorder
        std::array<int64_t, DIGITS> reordered{};
        for (int i = 0; i < DIGITS; i++) {
            int src_idx = forward ? i : (DIGITS - 1 - i);
            int dst_idx;
            
            if (forward) {
                // Forward: map i → (i * FIB[i % 14]) % DIGITS
                dst_idx = static_cast<int>((static_cast<uint64_t>(i) * FIB[i % 14] + nonce) % DIGITS);
            } else {
                // Reverse: inverse mapping
                dst_idx = static_cast<int>((static_cast<uint64_t>(DIGITS - 1 - i) * FIB[i % 14] + nonce) % DIGITS);
            }
            
            reordered[dst_idx] = chunks[src_idx];
        }
        
        // Reconstruct value
        int64_t result_int = 0;
        int64_t multiplier = 1;
        for (int i = 0; i < DIGITS; i++) {
            result_int += reordered[i] * multiplier;
            multiplier *= 1000000;
        }
        
        return static_cast<double>(result_int) / 1e6 + frac_part * PHI_INV;
    }
    
public:
    FibonacciReversalEngine() = default;
    void set_nonce(uint64_t nonce) { global_nonce_ = nonce; }
    
    // ═══ OBSERVE: Fibonacci scramble ═══
    std::pair<double, std::array<double, LAYERS>> 
    observe(double value, uint64_t operation_id = 0) {
        std::array<double, LAYERS> history{};
        
        if (operation_id == 0) operation_id = ++operation_counter_;
        uint64_t nonce = global_nonce_ ^ operation_id;
        
        double x = value;
        
        for (int i = 0; i < LAYERS; i++) {
            // Fibonacci scramble
            x = fib_scramble(x, nonce + i, true);
            
            // Add φ-scaled chaos
            double chaos = PHI * std::sin(x * PHI + FIB[i] * PHI_INV);
            history[i] = chaos;
            x = x + chaos;
        }
        
        return {x, history};
    }
    
    // ═══ UNOBSERVE: Reverse Fibonacci unscramble ═══
    double unobserve(double ciphertext, 
                     const std::array<double, LAYERS>& history,
                     uint64_t operation_id = 0) {
        if (operation_id == 0) operation_id = operation_counter_;
        uint64_t nonce = global_nonce_ ^ operation_id;
        
        double x = ciphertext;
        
        for (int i = LAYERS - 1; i >= 0; i--) {
            // Remove chaos
            x = x - history[i];
            
            // Fibonacci unscramble (reverse order)
            x = fib_scramble(x, nonce + i, false);
        }
        
        return x;
    }
    
    // ═══ METRICS ═══
    double reversal_depth() const { return LAYERS; }
};

} // namespace fibonacci_reversal
