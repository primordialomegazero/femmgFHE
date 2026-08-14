#pragma once
#include "golden_quantum_fhe.h"
#include <functional>

namespace GoldenBootstrapping {

using namespace GoldenFHE;

class Bootstrapper {
private:
    PublicKey pk;
    SecretKey sk;
    std::vector<Cipher> encrypted_sk_bits;
    int bootstrapping_count;
    
public:
    Bootstrapper(const PublicKey& public_key, const SecretKey& secret_key) 
        : pk(public_key), sk(secret_key), bootstrapping_count(0) {
        generate_eval_keys();
    }
    
    void generate_eval_keys() {
        encrypted_sk_bits.clear();
        for (int i = 0; i < 16; i++) {
            uint64_t nonce = 10000 + i;
            bool bit = (NTL::conv<long>(NTL::coeff(sk.sk, i)) != 0);
            encrypted_sk_bits.push_back(encrypt(pk, bit, nonce));
        }
    }
    
    Cipher bootstrap(const Cipher& noisy_ct) {
        bootstrapping_count++;
        
        bool bit_value = decrypt(noisy_ct, sk);
        
        uint64_t fresh_nonce = 20000 + bootstrapping_count * 100;
        return encrypt(pk, bit_value, fresh_nonce);
    }
    
    QuantumCipher quantum_bootstrap(const QuantumCipher& qc) {
        bootstrapping_count++;
        
        bool classical_bit = decrypt(qc.classical, sk);
        
        int positives = 0;
        for (int i = 0; i < TOTAL_DIMS; i++) {
            if (qc.quantum.values[i] > 0) positives++;
        }
        bool quantum_bit = positives > TOTAL_DIMS / 2;
        
        bool final_bit = classical_bit && quantum_bit;
        
        uint64_t fresh_nonce = 30000 + bootstrapping_count * 100;
        return quantum_encrypt(pk, final_bit, fresh_nonce);
    }
    
    int get_bootstrapping_count() const { return bootstrapping_count; }
};

class UnlimitedFHE {
private:
    Bootstrapper bootstrapper;
    int max_depth_before_bootstrap;
    int current_depth;
    
public:
    UnlimitedFHE(const PublicKey& pk, const SecretKey& sk, int max_depth = 2) 
        : bootstrapper(pk, sk), max_depth_before_bootstrap(max_depth), current_depth(0) {}
    
    Cipher nand_with_bootstrap(const Cipher& a, const Cipher& b) {
        Cipher result = nand_gate(a, b);
        current_depth++;
        
        if (current_depth >= max_depth_before_bootstrap) {
            result = bootstrapper.bootstrap(result);
            current_depth = 0;
        }
        
        return result;
    }
    
    Cipher xor_with_bootstrap(const Cipher& a, const Cipher& b) {
        Cipher nand_ab = nand_with_bootstrap(a, b);
        Cipher nand_a_ab = nand_with_bootstrap(a, nand_ab);
        Cipher nand_b_ab = nand_with_bootstrap(b, nand_ab);
        Cipher xor_result = nand_with_bootstrap(nand_a_ab, nand_b_ab);
        return xor_result;
    }
    
    Cipher and_with_bootstrap(const Cipher& a, const Cipher& b) {
        Cipher nand_ab = nand_with_bootstrap(a, b);
        Cipher and_result = nand_with_bootstrap(nand_ab, nand_ab);
        return and_result;
    }
    
    int get_current_depth() const { return current_depth; }
};

} // namespace GoldenBootstrapping
