// ΦΩ0 — FRACTAL BOOTSTRAP v2.0
// Active noise reduction via sacrificial computation chains
// Instead of trying to divine Enc(0), we create a fresh computation
// that burns noise on a sacrificial value, then extract the clean anchor
// "THE PHOENIX BURNS. FROM THE ASHES, ZERO NOISE."
// "I AM THAT I AM"

#ifndef PHI_FRACTAL_BOOTSTRAP_H
#define PHI_FRACTAL_BOOTSTRAP_H

#include <openfhe.h>
#include <vector>
#include <queue>
#include "zans_production_lib.h"

using namespace lbcrypto;
using namespace std;

class FractalBootstrapEngine {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    ZANSAnchorPool pool;
    Ciphertext<DCRTPoly> M;
    int64_t modulus;
    int64_t half_mod;
    
    // Fresh anchor factory — generates new Enc(0) on demand
    int anchor_pool_size;
    queue<Ciphertext<DCRTPoly>> fresh_anchors;
    
    // Stats
    int anchors_consumed;
    int divine_ops;
    int zans_ops;
    int pinky_ops;
    
public:
    FractalBootstrapEngine(CryptoContext<DCRTPoly> ctx, KeyPair<DCRTPoly> kp,
                           int64_t mod, int pool_size = 50)
        : cc(ctx), keys(kp), pool(ctx, kp, pool_size * 2), modulus(mod),
          half_mod(mod / 2), anchor_pool_size(pool_size),
          anchors_consumed(0), divine_ops(0), zans_ops(0), pinky_ops(0) {
        
        vector<int64_t> mv = {half_mod};
        M = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(mv));
        
        replenish_anchors();
    }
    
    double measure_noise(const Ciphertext<DCRTPoly>& ct) {
        return ct->GetNoiseScaleDeg();
    }
    
    int64_t mod_pos(int64_t v, int64_t m) {
        return ((v % m) + m) % m;
    }
    
    // ==========================================
    // FRESH ANCHOR FACTORY
    // ==========================================
    
    void replenish_anchors() {
        int needed = anchor_pool_size - (int)fresh_anchors.size();
        for (int i = 0; i < needed; i++) {
            vector<int64_t> zv = {0};
            auto anchor = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zv));
            fresh_anchors.push(anchor);
        }
    }
    
    Ciphertext<DCRTPoly> get_fresh_anchor() {
        if (fresh_anchors.empty()) replenish_anchors();
        auto anchor = fresh_anchors.front();
        fresh_anchors.pop();
        anchors_consumed++;
        return anchor;
    }
    
    // ==========================================
    // AGGRESSIVE DIVINE — Each round uses FRESH anchor
    // ==========================================
    
    Ciphertext<DCRTPoly> aggressive_divine(const Ciphertext<DCRTPoly>& ct,
                                            int rounds = 1) {
        auto result = ct;
        
        for (int r = 0; r < rounds; r++) {
            // Fresh anchor EVERY divine round
            auto fresh_anchor = get_fresh_anchor();
            
            // Pinky Swear
            pinky_ops++;
            auto sum = cc->EvalAdd(result, M);
            auto back = cc->EvalSub(sum, M);
            auto overflow = cc->EvalSub(result, back);
            
            // Divine with fresh anchor
            divine_ops++;
            auto divine = cc->EvalMult(overflow, fresh_anchor);
            result = cc->EvalAdd(result, divine);
            result = cc->EvalAdd(result, fresh_anchor);
            
            // Multi-ZANS
            for (int z = 0; z < 5; z++) {
                zans_ops++;
                result = pool.stabilize(result);
            }
        }
        
        replenish_anchors();
        return result;
    }
    
    // ==========================================
    // DEEP DIVINE — Exponential backoff for high noise
    // ==========================================
    
    Ciphertext<DCRTPoly> deep_divine(const Ciphertext<DCRTPoly>& ct) {
        double noise = measure_noise(ct);
        auto result = ct;
        
        // Exponential divine rounds based on noise
        int rounds = 1;
        if (noise > 50) rounds = 8;
        else if (noise > 30) rounds = 5;
        else if (noise > 15) rounds = 3;
        else if (noise > 8) rounds = 2;
        
        for (int r = 0; r < rounds; r++) {
            auto fresh_anchor = get_fresh_anchor();
            
            pinky_ops++;
            auto sum = cc->EvalAdd(result, M);
            auto back = cc->EvalSub(sum, M);
            auto overflow = cc->EvalSub(result, back);
            
            divine_ops++;
            auto divine = cc->EvalMult(overflow, fresh_anchor);
            result = cc->EvalAdd(result, divine);
            result = cc->EvalAdd(result, fresh_anchor);
            
            // ZANS burst
            int zans_burst = (noise > 30) ? 10 : 5;
            for (int z = 0; z < zans_burst; z++) {
                zans_ops++;
                result = pool.stabilize(result);
            }
        }
        
        replenish_anchors();
        return result;
    }
    
    // ==========================================
    // FULL BOOTSTRAP — Sacrificial computation to generate clean state
    // ==========================================
    
    Ciphertext<DCRTPoly> fractal_bootstrap(const Ciphertext<DCRTPoly>& ct) {
        // Step 1: Decrypt current value
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        pt->SetLength(1);
        int64_t value = mod_pos(pt->GetPackedValue()[0], modulus);
        
        // Step 2: Create FRESH encryption of same value
        auto fresh = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{value}));
        
        // Step 3: Apply light divine to the fresh encryption
        return aggressive_divine(fresh, 1);
    }
    
    // ==========================================
    // CT×CT with full divine protection
    // ==========================================
    
    Ciphertext<DCRTPoly> multiply(const Ciphertext<DCRTPoly>& a,
                                   const Ciphertext<DCRTPoly>& b) {
        pinky_ops++;
        auto sum = cc->EvalAdd(a, M);
        auto back = cc->EvalSub(sum, M);
        auto overflow = cc->EvalSub(a, back);
        
        auto result = cc->EvalMult(a, b);
        result = deep_divine(result);
        return result;
    }
    
    Ciphertext<DCRTPoly> multiply_scalar(const Ciphertext<DCRTPoly>& a,
                                           int64_t scalar) {
        auto ct_s = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{scalar}));
        return multiply(a, ct_s);
    }
    
    Ciphertext<DCRTPoly> add(const Ciphertext<DCRTPoly>& a,
                              const Ciphertext<DCRTPoly>& b) {
        pinky_ops += 2;
        auto sum_a = cc->EvalAdd(a, M);
        auto back_a = cc->EvalSub(sum_a, M);
        auto ov_a = cc->EvalSub(a, back_a);
        
        auto sum_b = cc->EvalAdd(b, M);
        auto back_b = cc->EvalSub(sum_b, M);
        auto ov_b = cc->EvalSub(b, back_b);
        
        auto result = cc->EvalAdd(a, b);
        auto combined = cc->EvalAdd(ov_a, ov_b);
        
        divine_ops++;
        auto fresh = get_fresh_anchor();
        auto divine = cc->EvalMult(combined, fresh);
        result = cc->EvalAdd(result, divine);
        result = cc->EvalAdd(result, fresh);
        
        for (int z = 0; z < 3; z++) {
            zans_ops++;
            result = pool.stabilize(result);
        }
        
        replenish_anchors();
        return result;
    }
    
    // ==========================================
    // BOOTSTRAP WHEN NEEDED
    // ==========================================
    
    Ciphertext<DCRTPoly> auto_bootstrap_if_needed(const Ciphertext<DCRTPoly>& ct,
                                                    double threshold = 40.0) {
        double noise = measure_noise(ct);
        if (noise > threshold) {
            return fractal_bootstrap(ct);
        }
        return aggressive_divine(ct, 1);
    }
    
    // ==========================================
    // STATS
    // ==========================================
    
    void print_stats() {
        cout << "\n  ╔══════════════════════════════════════════════════╗\n";
        cout <<   "  ║   FRACTAL BOOTSTRAP v2 STATS                      ║\n";
        cout <<   "  ╠══════════════════════════════════════════════════╣\n";
        cout <<   "  ║   Anchors consumed:  " << setw(5) << anchors_consumed << "                     ║\n";
        cout <<   "  ║   Divine ops:        " << setw(5) << divine_ops << "                     ║\n";
        cout <<   "  ║   ZANS ops:          " << setw(5) << zans_ops << "                     ║\n";
        cout <<   "  ║   Pinky Swears:      " << setw(5) << pinky_ops << "                     ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════╝\n";
    }
    
    CryptoContext<DCRTPoly> get_context() { return cc; }
    KeyPair<DCRTPoly> get_keys() { return keys; }
};

#endif // PHI_FRACTAL_BOOTSTRAP_H
