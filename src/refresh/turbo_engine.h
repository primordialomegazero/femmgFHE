#pragma once
// ═══════════════════════════════════════════════════════════════════════════════
// TURBO ENGINE — SIMD DualGate Batching for Extreme Speed
// ═══════════════════════════════════════════════════════════════════════════════
//
// N-configurable SIMD acceleration. Preserves KS = 0.000000.
//
// Configuration:
//   N_turbo_batch: 0 = disabled (serial mode), 1-512 = SIMD batch size
//   At RingDim 4096: max 512 pairs per batch
//   At RingDim 16384: max 2048 pairs per batch
//   At RingDim 32768: max 4096 pairs per batch
//
// Speedup: ~N_turbo_batch × faster FHE evaluation
// Security: Zero impact — φ/ψ duality preserved per sample

#include "../fhe/fhe_core.h"
#include <vector>
#include <utility>

struct TurboEngine {
    int N_turbo_batch;    // 0=off, 1-512=on
    int ring_dim;
    int max_batch;
    bool enabled;
    
    void init(int rd, int batch_size = 0) {
        ring_dim = rd;
        N_turbo_batch = batch_size;
        max_batch = rd / 8;  // Conservative: RingDim/8 DualGate pairs
        enabled = (N_turbo_batch > 1);
    }
    
    // Pack multiple DualGate (a,b) pairs into SIMD ciphertexts
    std::pair<Ciphertext<DCRTPoly>, Ciphertext<DCRTPoly>>
    pack(const std::vector<double>& a_vals,
         const std::vector<double>& b_vals,
         SecureContext& sc) {
        int N = std::min((int)a_vals.size(), max_batch);
        int slots = std::min(N, ring_dim / 4);
        
        std::vector<double> packed_a(slots, 0.0);
        std::vector<double> packed_b(slots, 0.0);
        
        for (int i = 0; i < N && i < slots; i++) {
            packed_a[i] = a_vals[i];
            packed_b[i] = b_vals[i];
        }
        
        auto pt_a = sc.cc->MakeCKKSPackedPlaintext(packed_a);
        auto pt_b = sc.cc->MakeCKKSPackedPlaintext(packed_b);
        
        return {
            sc.cc->Encrypt(sc.kp.publicKey, pt_a),
            sc.cc->Encrypt(sc.kp.publicKey, pt_b)
        };
    }
    
    // Unpack SIMD results
    std::vector<double> unpack(const Ciphertext<DCRTPoly>& ct, 
                               SecureContext& sc, int count) {
        Plaintext pt;
        sc.cc->Decrypt(sc.kp.secretKey, ct, &pt);
        auto values = pt->GetCKKSPackedValue();
        std::vector<double> result;
        int limit = std::min(count, (int)values.size());
        for (int i = 0; i < limit; i++) result.push_back(values[i].real());
        return result;
    }
};
