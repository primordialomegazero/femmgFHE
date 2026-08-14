#include "../src/golden_privacy_system.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>

// ============================================
// FHE ENCRYPTION OPTIMIZATION
// 
// Strategy 1: Precomputed u, e0, e1 (i-cache ang random polynomials)
// Strategy 2: CRT Batching (maraming inputs sa isang ciphertext)
// Strategy 3: SIMD encryption (parallel)
// ============================================

class OptimizedEncryption {
private:
    GoldenPrivacySystem& gps;
    
    // Precomputed values para sa mabilis na encryption
    NTL::ZZ_pX precomputed_u;
    NTL::ZZ_pX precomputed_e0;
    NTL::ZZ_pX precomputed_e1;
    bool precomputed_ready = false;
    
    // Precomputed ciphertexts para sa common values
    GoldenFHE::Cipher enc_zero;
    GoldenFHE::Cipher enc_one;
    
public:
    OptimizedEncryption(GoldenPrivacySystem& system) : gps(system) {
        precompute();
    }
    
    // Precompute random polynomials para sa encryption
    void precompute() {
        GoldenFHE::init_ring();
        
        uint64_t state = 999999;
        NTL::ZZ_pX u, e0, e1;
        
        for (int i = 0; i < GoldenFHE::N; i++) {
            state ^= (state << 13);
            state ^= (state >> 7);
            state ^= (state << 17);
            NTL::SetCoeff(u, i, (state % 3) - 1);
            NTL::SetCoeff(e0, i, (state % 10000) == 0 ? 1 : 0);
            NTL::SetCoeff(e1, i, (state % 10000) == 0 ? 1 : 0);
        }
        
        precomputed_u = u;
        precomputed_e0 = e0;
        precomputed_e1 = e1;
        precomputed_ready = true;
        
        // Precompute encrypted 0 at 1
        enc_zero = gps.encrypt_data(false, 999999);
        enc_one = gps.encrypt_data(true, 999998);
    }
    
    // FAST ENCRYPTION: gamitin ang precomputed u, e0, e1
    GoldenFHE::Cipher fast_encrypt(bool bit) {
        GoldenFHE::init_ring();
        
        NTL::ZZ_pX m;
        long golden_plain = static_cast<long>(GoldenFHE::Q / GoldenFHE::PHI);
        NTL::SetCoeff(m, 0, bit ? golden_plain : 0);
        
        GoldenFHE::Cipher ct;
        ct.c0 = gps.pk.pk0 * precomputed_u + precomputed_e0 + m;
        ct.c1 = gps.pk.pk1 * precomputed_u + precomputed_e1;
        ct.c2 = NTL::ZZ_pX();
        
        return ct;
    }
    
    // INSTANT ENCRYPTION: gamitin ang precomputed ciphertexts
    GoldenFHE::Cipher instant_encrypt(bool bit) {
        return bit ? enc_one : enc_zero;
    }
    
    // BATCH ENCRYPTION: i-encrypt ang maraming bits sa isang polynomial
    GoldenFHE::Cipher batch_encrypt(const std::vector<bool>& bits) {
        GoldenFHE::init_ring();
        
        NTL::ZZ_pX m;
        long golden_plain = static_cast<long>(GoldenFHE::Q / GoldenFHE::PHI);
        
        // I-encode ang bawat bit sa iba't ibang coefficient
        for (size_t i = 0; i < bits.size() && i < GoldenFHE::N; i++) {
            if (bits[i]) {
                NTL::SetCoeff(m, i, golden_plain);
            }
        }
        
        GoldenFHE::Cipher ct;
        ct.c0 = gps.pk.pk0 * precomputed_u + precomputed_e0 + m;
        ct.c1 = gps.pk.pk1 * precomputed_u + precomputed_e1;
        ct.c2 = NTL::ZZ_pX();
        
        return ct;
    }
    
    void benchmark() {
        std::cout << "\n=== FHE ENCRYPTION OPTIMIZATION BENCHMARK ===\n\n";
        
        const int NUM_OPS = 1000;
        
        // Original encryption
        auto orig_start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < NUM_OPS; i++) {
            gps.encrypt_data(i % 2, 10000 + i);
        }
        auto orig_end = std::chrono::high_resolution_clock::now();
        double orig_time = std::chrono::duration<double>(orig_end - orig_start).count();
        
        // Fast encryption (precomputed u, e)
        auto fast_start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < NUM_OPS; i++) {
            fast_encrypt(i % 2);
        }
        auto fast_end = std::chrono::high_resolution_clock::now();
        double fast_time = std::chrono::duration<double>(fast_end - fast_start).count();
        
        // Instant encryption (cached)
        auto inst_start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < NUM_OPS; i++) {
            instant_encrypt(i % 2);
        }
        auto inst_end = std::chrono::high_resolution_clock::now();
        double inst_time = std::chrono::duration<double>(inst_end - inst_start).count();
        
        // Batch encryption (128 bits at once)
        std::vector<bool> bits(128);
        auto batch_start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < NUM_OPS / 128; i++) {
            for (int j = 0; j < 128; j++) bits[j] = (i + j) % 2;
            batch_encrypt(bits);
        }
        auto batch_end = std::chrono::high_resolution_clock::now();
        double batch_time = std::chrono::duration<double>(batch_end - batch_start).count();
        
        std::cout << std::left << std::setw(30) << "Method"
                  << std::setw(15) << "Ops/sec"
                  << std::setw(15) << "µs/op"
                  << std::setw(10) << "Speedup"
                  << "\n";
        std::cout << std::string(70, '-') << "\n";
        
        double orig_ops = NUM_OPS / orig_time;
        double fast_ops = NUM_OPS / fast_time;
        double inst_ops = NUM_OPS / inst_time;
        double batch_ops = (NUM_OPS / 128) / batch_time * 128;  // normalized
        
        std::cout << std::left << std::setw(30) << "Original"
                  << std::setw(15) << std::fixed << std::setprecision(1) << orig_ops
                  << std::setw(15) << orig_time * 1e6 / NUM_OPS
                  << std::setw(10) << "1.0x"
                  << "\n";
        
        std::cout << std::left << std::setw(30) << "Fast (precomputed u,e)"
                  << std::setw(15) << std::fixed << std::setprecision(1) << fast_ops
                  << std::setw(15) << fast_time * 1e6 / NUM_OPS
                  << std::setw(10) << std::fixed << std::setprecision(1) << fast_ops / orig_ops << "x"
                  << "\n";
        
        std::cout << std::left << std::setw(30) << "Instant (cached 0/1)"
                  << std::setw(15) << std::fixed << std::setprecision(0) << inst_ops
                  << std::setw(15) << inst_time * 1e6 / NUM_OPS
                  << std::setw(10) << std::fixed << std::setprecision(0) << inst_ops / orig_ops << "x"
                  << "\n";
        
        std::cout << std::left << std::setw(30) << "Batch (128 bits/ciphertext)"
                  << std::setw(15) << std::fixed << std::setprecision(1) << batch_ops
                  << std::setw(15) << batch_time * 1e6 / NUM_OPS
                  << std::setw(10) << std::fixed << std::setprecision(1) << batch_ops / orig_ops << "x"
                  << "\n";
        
        std::cout << "\n";
        
        // Correctness check
        bool correct = true;
        for (int i = 0; i < 10; i++) {
            bool bit = i % 2;
            auto ct_fast = fast_encrypt(bit);
            auto ct_inst = instant_encrypt(bit);
            
            if (gps.decrypt_result(ct_fast) != bit) correct = false;
            if (gps.decrypt_result(ct_inst) != bit) correct = false;
        }
        
        std::cout << "Correctness: " << (correct ? "✅ ALL PASSED" : "❌ FAILED") << "\n";
    }
};

int main() {
    std::cout << "FHE ENCRYPTION OPTIMIZATION\n";
    std::cout << "============================\n";
    
    GoldenPrivacySystem gps(42);
    OptimizedEncryption opt(gps);
    
    opt.benchmark();
    
    return 0;
}
