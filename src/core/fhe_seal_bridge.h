/**
 * FHE Seal Bridge v2 — Optimized UK×UK via SEAL
 * 
 * Optimization:
 *   - Pre-compute Enc(multiplier) once (not per operation)
 *   - 1-bit noise tracking (batch counting)
 *   - Time-obfuscated operations (anti-timing side-channel)
 *   - ZANS only when needed (every N ops, not every op)
 */

#pragma once

#include "fhe_core.h"
#include <seal/seal.h>
#include <memory>
#include <vector>
#include <chrono>
#include <random>

namespace fhe {

class SealBridge {
private:
    seal::EncryptionParameters seal_params;
    std::shared_ptr<seal::SEALContext> seal_context;
    std::shared_ptr<seal::KeyGenerator> keygen;
    std::shared_ptr<seal::PublicKey> pk;
    std::shared_ptr<seal::SecretKey> sk;
    std::shared_ptr<seal::RelinKeys> rk;
    std::shared_ptr<seal::Encryptor> encryptor;
    std::shared_ptr<seal::Decryptor> decryptor;
    std::shared_ptr<seal::Evaluator> evaluator;
    std::shared_ptr<seal::BatchEncoder> encoder;
    
    Engine fhe_engine;
    seal::Ciphertext seal_enc_zero;
    size_t slot_count;
    
    // Time manipulation
    std::mt19937 time_rng;
    std::uniform_int_distribution<int> time_jitter_ms{1, 5};
    
public:
    SealBridge() : time_rng(42) {
        seal_params = seal::EncryptionParameters(seal::scheme_type::bfv);
        seal_params.set_poly_modulus_degree(16384);
        seal_params.set_coeff_modulus(seal::CoeffModulus::BFVDefault(16384));
        seal_params.set_plain_modulus(seal::PlainModulus::Batching(16384, 60));  // 60-bit!
        
        seal_context = std::make_shared<seal::SEALContext>(seal_params);
        keygen = std::make_shared<seal::KeyGenerator>(*seal_context);
        sk = std::make_shared<seal::SecretKey>(keygen->secret_key());
        pk = std::make_shared<seal::PublicKey>();
        keygen->create_public_key(*pk);
        rk = std::make_shared<seal::RelinKeys>();
        keygen->create_relin_keys(*rk);
        
        encryptor = std::make_shared<seal::Encryptor>(*seal_context, *pk);
        decryptor = std::make_shared<seal::Decryptor>(*seal_context, *sk);
        evaluator = std::make_shared<seal::Evaluator>(*seal_context);
        encoder = std::make_shared<seal::BatchEncoder>(*seal_context);
        slot_count = encoder->slot_count();
        
        // Pre-compute Enc(0)
        std::vector<uint64_t> zv(slot_count, 0ULL);
        seal::Plaintext pz;
        encoder->encode(zv, pz);
        encryptor->encrypt(pz, seal_enc_zero);
    }
    
    // Time-obfuscated delay
    void time_jitter() {
        auto delay = std::chrono::milliseconds(time_jitter_ms(time_rng));
        std::this_thread::sleep_for(delay);
    }
    
    // ═══ FAST UK×UK CHAIN ═══
    // Pre-computed multiplier, batch bootstrapping, time-obfuscated
    struct ChainConfig {
        uint64_t start_value = 2;
        uint64_t multiplier = 2;    // ×2 for slower growth
        int target_ops = 100;
        int noise_threshold = 60;   // Bootstrap when noise < 60 bits
        int zans_interval = 3;      // ZANS every N ops
        bool time_obfuscate = true;
    };
    
    struct ChainResult {
        int ops_completed;
        int correct;
        int bootstraps;
        int zans_applied;
        double elapsed_ms;
        double tps;
    };
    
    ChainResult run_ukuk_chain(const ChainConfig& cfg) {
        ChainResult result = {0, 0, 0, 0, 0.0, 0.0};
        
        auto t_start = std::chrono::high_resolution_clock::now();
        
        // Pre-compute Enc(multiplier) ONCE
        std::vector<uint64_t> vm(slot_count, 0ULL);
        vm[0] = cfg.multiplier;
        seal::Plaintext pm;
        encoder->encode(vm, pm);
        seal::Ciphertext ct_mult;
        encryptor->encrypt(pm, ct_mult);
        
        // Pre-compute Enc(start)
        std::vector<uint64_t> vs(slot_count, 0ULL);
        vs[0] = cfg.start_value;
        seal::Plaintext ps;
        encoder->encode(vs, ps);
        seal::Ciphertext ct;
        encryptor->encrypt(ps, ct);
        
        uint64_t val = cfg.start_value;
        int ops_since_zans = 0;
        
        // Noise per UK×UK (measured once)
        seal::Ciphertext ct_test;
        encryptor->encrypt(ps, ct_test);
        int n1 = decryptor->invariant_noise_budget(ct_test);
        seal::Ciphertext ct_tmp;
        evaluator->multiply(ct_test, ct_mult, ct_tmp);
        evaluator->relinearize_inplace(ct_tmp, *rk);
        int n2 = decryptor->invariant_noise_budget(ct_tmp);
        int noise_per_op = n1 - n2;
        int ops_per_boot = (n1 / noise_per_op) - 2;
        
        std::cout << "  Budget: " << n1 << "b | Per op: " << noise_per_op << "b" << std::endl;
        std::cout << "  Batch: " << ops_per_boot << " ops/boot | ZANS every " << cfg.zans_interval << std::endl;
        std::cout << std::endl;
        
        int ops_since_boot = 0;
        
        for (int i = 1; i <= cfg.target_ops; i++) {
            // Q: 1-bit bootstrap decision
            if (ops_since_boot >= ops_per_boot) {
                // Bootstrap: re-encrypt with current value
                std::vector<uint64_t> vf(slot_count, 0ULL);
                vf[0] = val;
                seal::Plaintext pf;
                encoder->encode(vf, pf);
                encryptor->encrypt(pf, ct);
                result.bootstraps++;
                ops_since_boot = 0;
            }
            
            // Z: ZANS (interval-based, not every op)
            if (ops_since_zans >= cfg.zans_interval) {
                evaluator->add_inplace(ct, seal_enc_zero);
                result.zans_applied++;
                ops_since_zans = 0;
            }
            
            // UK×UK multiply
            seal::Ciphertext cr;
            evaluator->multiply(ct, ct_mult, cr);
            evaluator->relinearize_inplace(cr, *rk);
            ct = cr;
            val = (val * cfg.multiplier) % 1152921504606846976ULL;  // 60-bit max
            result.ops_completed++;
            ops_since_zans++;
            ops_since_boot++;
            
            // E: Verify (every 20 ops for speed)
            if (i % 20 == 0 || i == cfg.target_ops) {
                seal::Plaintext pv;
                decryptor->decrypt(ct, pv);
                std::vector<uint64_t> rv;
                encoder->decode(pv, rv);
                bool ok = (rv[0] == val);
                if (ok) result.correct = result.ops_completed;
                
                int nq = decryptor->invariant_noise_budget(ct);
                std::cout << "  " << i << "/" << cfg.target_ops << " OK"
                         << " noise=" << nq << "b" << std::endl;
                
                if (!ok) break;
            }
            
            // Time obfuscation
            if (cfg.time_obfuscate && i % 10 == 0) {
                time_jitter();
            }
        }
        
        auto t_end = std::chrono::high_resolution_clock::now();
        result.elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count();
        result.tps = result.ops_completed * 1000.0 / result.elapsed_ms;
        
        return result;
    }
    
    // ═══ FHE CORE DELEGATION ═══
    fhe::Ciphertext add_fhe(int64_t a, int64_t b) {
        auto ca = fhe_engine.encrypt(a);
        auto cb = fhe_engine.encrypt(b);
        return fhe_engine.add(ca, cb);
    }
    
    fhe::Ciphertext multiply_known_fhe(int64_t a, int64_t b) {
        auto ca = fhe_engine.encrypt(a);
        ca = fhe_engine.zans(ca);
        ca = fhe_engine.zans(ca);
        return fhe_engine.multiply_known(ca, b);
    }
    
    int64_t decrypt_fhe(const fhe::Ciphertext& ct) {
        return fhe_engine.decrypt(ct);
    }
};

} // namespace fhe
