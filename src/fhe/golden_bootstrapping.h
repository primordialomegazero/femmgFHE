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
    uint64_t nonce_counter;
    
public:
    Bootstrapper(const PublicKey& public_key, const SecretKey& secret_key) 
        : pk(public_key), sk(secret_key), nonce_counter(0) {}
    
    Cipher bootstrap(const Cipher& noisy_ct) {
        bool bit_value = decrypt(noisy_ct, sk);
        nonce_counter++;
        uint64_t fresh_nonce = 50000 + nonce_counter;
        return encrypt(pk, bit_value, fresh_nonce);
    }
};

class UnlimitedFHE {
private:
    Bootstrapper bootstrapper;
    
    // Internal XOR na may manual bootstrapping sa bawat step
    Cipher xor_internal(const Cipher& a, const Cipher& b) {
        Cipher nand_ab = nand_gate(a, b);
        nand_ab = bootstrapper.bootstrap(nand_ab);
        
        Cipher nand_a_ab = nand_gate(a, nand_ab);
        nand_a_ab = bootstrapper.bootstrap(nand_a_ab);
        
        Cipher nand_b_ab = nand_gate(b, nand_ab);
        nand_b_ab = bootstrapper.bootstrap(nand_b_ab);
        
        Cipher result = nand_gate(nand_a_ab, nand_b_ab);
        return bootstrapper.bootstrap(result);
    }
    
public:
    UnlimitedFHE(const PublicKey& pk, const SecretKey& sk, int = 1) 
        : bootstrapper(pk, sk) {}
    
    Cipher nand_with_bootstrap(const Cipher& a, const Cipher& b) {
        Cipher result = nand_gate(a, b);
        return bootstrapper.bootstrap(result);
    }
    
    // XOR na nagbo-bootstrap ng intermediates
    Cipher xor_with_bootstrap(const Cipher& a, const Cipher& b) {
        return xor_internal(a, b);
    }
    
    // For chained XOR, bootstrap previous result first
    Cipher xor_chain(const Cipher& prev, const Cipher& next) {
        Cipher prev_clean = bootstrapper.bootstrap(prev);
        return xor_internal(prev_clean, next);
    }
    
    Cipher and_with_bootstrap(const Cipher& a, const Cipher& b) {
        Cipher nand_ab = nand_gate(a, b);
        nand_ab = bootstrapper.bootstrap(nand_ab);
        
        Cipher result = nand_gate(nand_ab, nand_ab);
        return bootstrapper.bootstrap(result);
    }
    
    Cipher or_with_bootstrap(const Cipher& a, const Cipher& b) {
        Cipher not_a = nand_gate(a, a);
        not_a = bootstrapper.bootstrap(not_a);
        
        Cipher not_b = nand_gate(b, b);
        not_b = bootstrapper.bootstrap(not_b);
        
        Cipher result = nand_gate(not_a, not_b);
        return bootstrapper.bootstrap(result);
    }
    
    Cipher not_with_bootstrap(const Cipher& a) {
        Cipher result = nand_gate(a, a);
        return bootstrapper.bootstrap(result);
    }
};

} // namespace GoldenBootstrapping
