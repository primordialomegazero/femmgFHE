#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <iomanip>

// I-extract ang noise value mula sa ciphertext
long get_noise(const GoldenFHE::Cipher& ct, const GoldenFHE::SecretKey& sk) {
    NTL::ZZ_pX s = sk.sk;
    NTL::ZZ_pX s2 = s * s;
    NTL::ZZ_pX noise = ct.c0 + ct.c1 * s + ct.c2 * s2;
    return NTL::conv<long>(NTL::coeff(noise, 0));
}

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    std::cout << "Noise Behavior + Value Preservation (1K ops)...\n";
    std::cout << "===============================================\n\n";
    
    long golden_plain = static_cast<long>(GoldenFHE::Q / GoldenFHE::PHI);
    long threshold = static_cast<long>(GoldenFHE::Q / (2 * GoldenFHE::PHI));
    
    std::cout << "Golden plain (encrypted 1): " << golden_plain << "\n";
    std::cout << "Threshold: " << threshold << "\n";
    std::cout << "Q: " << GoldenFHE::Q << "\n\n";
    
    // I-encrypt ang value 1
    GoldenFHE::Cipher val = GoldenFHE::encrypt(pk, true, 1000000);
    
    std::cout << std::left << std::setw(6) << "Op"
              << std::setw(12) << "Noise"
              << std::setw(14) << "Distance"
              << std::setw(10) << "Decoded"
              << std::setw(10) << "Expected"
              << std::setw(10) << "Status"
              << "\n";
    std::cout << std::string(62, '-') << "\n";
    
    for (int i = 0; i < 1000; i++) {
        static GoldenBootstrapping::Bootstrapper boot(pk, sk);
        static GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
        
        // NOT operation na may bootstrapping
        val = fhe.not_with_bootstrap(val);
        
        long noise = get_noise(val, sk);
        bool decoded = GoldenFHE::decrypt(val, sk);
        bool expected = ((i + 1) % 2 == 0);
        
        // Distance sa nearest valid value
        long target = expected ? golden_plain : 0;
        long distance = std::abs(noise - target);
        
        if ((i + 1) % 100 == 0) {
            std::cout << std::left << std::setw(6) << (i + 1)
                      << std::setw(12) << noise
                      << std::setw(14) << distance
                      << std::setw(10) << (decoded ? "1" : "0")
                      << std::setw(10) << (expected ? "1" : "0")
                      << std::setw(10) << (decoded == expected ? "OK" : "FAIL")
                      << "\n";
        }
    }
    
    std::cout << "\n=== NOISE STATISTICS ===\n";
    
    // Test iba't ibang arbitrary operations
    std::cout << "\n=== ARBITRARY OPERATIONS ===\n";
    
    // Test: paano nagbabago ang noise sa iba't ibang gates
    GoldenFHE::Cipher a = GoldenFHE::encrypt(pk, true, 2000000);
    GoldenFHE::Cipher b = GoldenFHE::encrypt(pk, false, 2000001);
    
    GoldenBootstrapping::Bootstrapper boot2(pk, sk);
    
    // AND
    GoldenFHE::Cipher and_result = nand_gate(nand_gate(a, b), nand_gate(a, b));
    long noise_and = get_noise(and_result, sk);
    std::cout << "AND(1,0): noise=" << noise_and 
              << " decoded=" << GoldenFHE::decrypt(and_result, sk) 
              << " (expected 0)\n";
    
    // OR
    GoldenFHE::Cipher not_a = nand_gate(a, a);
    GoldenFHE::Cipher not_b = nand_gate(b, b);
    GoldenFHE::Cipher or_result = nand_gate(not_a, not_b);
    long noise_or = get_noise(or_result, sk);
    std::cout << "OR(1,0): noise=" << noise_or 
              << " decoded=" << GoldenFHE::decrypt(or_result, sk) 
              << " (expected 1)\n";
    
    // XOR
    GoldenFHE::Cipher xor_result = GoldenFHE::XOR(a, b);
    long noise_xor = get_noise(xor_result, sk);
    std::cout << "XOR(1,0): noise=" << noise_xor 
              << " decoded=" << GoldenFHE::decrypt(xor_result, sk) 
              << " (expected 1)\n";
    
    std::cout << "\n✅ NOISE VISUALIZATION COMPLETE!\n";
    return 0;
}
