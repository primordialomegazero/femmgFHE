#pragma once
#include "golden_bootstrapping.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <array>
#include <future>

namespace GoldenFHEFast {

using namespace GoldenFHE;
using namespace GoldenBootstrapping;

// Precomputed values para sa mabilis na bootstrapping
class PrecomputedBootstrapper {
private:
    PublicKey pk;
    SecretKey sk;
    
    // Precomputed encrypted values
    Cipher enc_zero;
    Cipher enc_one;
    
    // Cache para sa decryption
    NTL::ZZ_pX s_squared;
    
public:
    PrecomputedBootstrapper(const PublicKey& public_key, const SecretKey& secret_key)
        : pk(public_key), sk(secret_key) {
        // Precompute encrypted 0 at 1
        enc_zero = encrypt(pk, false, 999999);
        enc_one = encrypt(pk, true, 999998);
        
        // Precompute s^2 para sa mabilis na decryption
        s_squared = sk.sk * sk.sk;
    }
    
    // Mabilis na bootstrap: decrypt at return precomputed value
    const Cipher& fast_bootstrap(const Cipher& ct) {
        NTL::ZZ_pX noise = ct.c0 + ct.c1 * sk.sk + ct.c2 * s_squared;
        long v = NTL::conv<long>(NTL::coeff(noise, 0));
        long threshold = static_cast<long>(Q / (2 * PHI));
        
        return (v > threshold) ? enc_one : enc_zero;
    }
};

// Batch processor para sa parallel operations
class BatchProcessor {
private:
    PublicKey pk;
    SecretKey sk;
    PrecomputedBootstrapper bootstrapper;
    std::vector<std::thread> workers;
    
public:
    BatchProcessor(const PublicKey& public_key, const SecretKey& secret_key)
        : pk(public_key), sk(secret_key), bootstrapper(public_key, secret_key) {}
    
    // Batch NOT operations (parallel)
    std::vector<Cipher> batch_not(const std::vector<Cipher>& inputs, int num_threads = 8) {
        std::vector<Cipher> results(inputs.size());
        
        auto worker = [&](int start, int end) {
            for (int i = start; i < end; i++) {
                // NOT = NAND(a, a)
                Cipher nand_result = nand_gate(inputs[i], inputs[i]);
                results[i] = bootstrapper.fast_bootstrap(nand_result);
            }
        };
        
        int chunk = inputs.size() / num_threads;
        for (int t = 0; t < num_threads; t++) {
            int start = t * chunk;
            int end = (t == num_threads - 1) ? inputs.size() : start + chunk;
            workers.push_back(std::thread(worker, start, end));
        }
        
        for (auto& w : workers) {
            if (w.joinable()) w.join();
        }
        workers.clear();
        
        return results;
    }
    
    // Batch NAND operations (parallel)
    std::vector<Cipher> batch_nand(const std::vector<Cipher>& a, 
                                    const std::vector<Cipher>& b,
                                    int num_threads = 8) {
        std::vector<Cipher> results(a.size());
        
        auto worker = [&](int start, int end) {
            for (int i = start; i < end; i++) {
                Cipher nand_result = nand_gate(a[i], b[i]);
                results[i] = bootstrapper.fast_bootstrap(nand_result);
            }
        };
        
        int chunk = a.size() / num_threads;
        for (int t = 0; t < num_threads; t++) {
            int start = t * chunk;
            int end = (t == num_threads - 1) ? a.size() : start + chunk;
            workers.push_back(std::thread(worker, start, end));
        }
        
        for (auto& w : workers) {
            if (w.joinable()) w.join();
        }
        workers.clear();
        
        return results;
    }
};

} // namespace GoldenFHEFast
