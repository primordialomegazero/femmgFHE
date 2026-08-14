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
    
public:
    UnlimitedFHE(const PublicKey& pk, const SecretKey& sk, int = 1) 
        : bootstrapper(pk, sk) {}
    
    // NAND na may bootstrapping ng result LANG
    Cipher nand_with_bootstrap(const Cipher& a, const Cipher& b) {
        Cipher result = nand_gate(a, b);
        return bootstrapper.bootstrap(result);
    }
    
    Cipher xor_with_bootstrap(const Cipher& a, const Cipher& b) {
        Cipher nand_ab = nand_with_bootstrap(a, b);
        Cipher nand_a_ab = nand_with_bootstrap(a, nand_ab);
        Cipher nand_b_ab = nand_with_bootstrap(b, nand_ab);
        return nand_with_bootstrap(nand_a_ab, nand_b_ab);
    }
    
    Cipher and_with_bootstrap(const Cipher& a, const Cipher& b) {
        Cipher nand_ab = nand_with_bootstrap(a, b);
        return nand_with_bootstrap(nand_ab, nand_ab);
    }
    
    Cipher or_with_bootstrap(const Cipher& a, const Cipher& b) {
        Cipher not_a = nand_with_bootstrap(a, a);
        Cipher not_b = nand_with_bootstrap(b, b);
        return nand_with_bootstrap(not_a, not_b);
    }
    
    Cipher not_with_bootstrap(const Cipher& a) {
        return nand_with_bootstrap(a, a);
    }
};

} // namespace GoldenBootstrapping
