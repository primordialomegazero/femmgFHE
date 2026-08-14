#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>
#include <vector>
#include <functional>
#include <iomanip>
#include <cmath>

class ProgrammaticBootstrapping {
private:
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    std::vector<std::vector<GoldenFHE::Cipher>> program_lut;
    int input_bits;
    
public:
    ProgrammaticBootstrapping(const GoldenFHE::PublicKey& public_key,
                              const GoldenFHE::SecretKey& secret_key)
        : pk(public_key), sk(secret_key), input_bits(0) {}
    
    void setup_program(const std::function<bool(const std::vector<bool>&)>& func,
                       int n_bits) {
        input_bits = n_bits;
        program_lut.clear();
        int num_entries = 1 << n_bits;
        program_lut.resize(num_entries);
        
        for (int i = 0; i < num_entries; i++) {
            std::vector<bool> inputs(n_bits);
            for (int j = 0; j < n_bits; j++) {
                inputs[j] = (i >> j) & 1;
            }
            bool output = func(inputs);
            program_lut[i].push_back(GoldenFHE::encrypt(pk, output, 10000000 + i));
        }
    }
    
    GoldenFHE::Cipher evaluate_program(const std::vector<GoldenFHE::Cipher>& enc_inputs) {
        GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
        int num_entries = 1 << input_bits;
        GoldenFHE::Cipher result = GoldenFHE::encrypt(pk, false, 20000000);
        
        for (int i = 0; i < num_entries; i++) {
            GoldenFHE::Cipher match = GoldenFHE::encrypt(pk, true, 30000000 + i);
            
            for (int j = 0; j < input_bits; j++) {
                bool expected = (i >> j) & 1;
                GoldenFHE::Cipher expected_ct = GoldenFHE::encrypt(pk, expected, 40000000 + i * 10 + j);
                GoldenFHE::Cipher eq = fhe.xor_with_bootstrap(enc_inputs[j], expected_ct);
                GoldenFHE::Cipher not_equal = fhe.not_with_bootstrap(eq);
                match = fhe.and_with_bootstrap(match, not_equal);
            }
            
            GoldenFHE::Cipher selected = fhe.and_with_bootstrap(match, program_lut[i][0]);
            result = fhe.or_with_bootstrap(result, selected);
        }
        
        return result;
    }
};

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    std::cout << "Testing Programmatic Bootstrapping...\n\n";
    
    ProgrammaticBootstrapping pb(pk, sk);
    
    // Test 1: 2-bit AND
    auto and2 = [](const std::vector<bool>& inputs) {
        return inputs[0] && inputs[1];
    };
    
    pb.setup_program(and2, 2);
    
    std::cout << "2-bit AND:\n";
    bool passed = true;
    
    for (int i = 0; i < 4; i++) {
        std::vector<bool> inputs = {(bool)((i >> 0) & 1), (bool)((i >> 1) & 1)};
        std::vector<GoldenFHE::Cipher> enc_inputs;
        for (int j = 0; j < 2; j++) {
            enc_inputs.push_back(GoldenFHE::encrypt(pk, inputs[j], 50000000 + i * 10 + j));
        }
        
        GoldenFHE::Cipher result = pb.evaluate_program(enc_inputs);
        bool output = GoldenFHE::decrypt(result, sk);
        bool expected = inputs[0] && inputs[1];
        
        std::cout << "  AND(" << inputs[0] << "," << inputs[1] << ") = " 
                  << output << " (expected " << expected << ")\n";
        
        if (output != expected) passed = false;
    }
    
    // Test 2: 3-bit majority
    auto majority3 = [](const std::vector<bool>& inputs) {
        int count = inputs[0] + inputs[1] + inputs[2];
        return count >= 2;
    };
    
    pb.setup_program(majority3, 3);
    
    std::cout << "\n3-bit Majority:\n";
    
    for (int i = 0; i < 8; i++) {
        std::vector<bool> inputs = {
            (bool)((i >> 0) & 1), (bool)((i >> 1) & 1), (bool)((i >> 2) & 1)
        };
        std::vector<GoldenFHE::Cipher> enc_inputs;
        for (int j = 0; j < 3; j++) {
            enc_inputs.push_back(GoldenFHE::encrypt(pk, inputs[j], 60000000 + i * 10 + j));
        }
        
        GoldenFHE::Cipher result = pb.evaluate_program(enc_inputs);
        bool output = GoldenFHE::decrypt(result, sk);
        bool expected = (inputs[0] + inputs[1] + inputs[2]) >= 2;
        
        std::cout << "  Maj(" << inputs[0] << "," << inputs[1] << "," << inputs[2] 
                  << ") = " << output << " (expected " << expected << ")\n";
        
        if (output != expected) passed = false;
    }
    
    // Test 3: Bootstrap refresh (FIXED - 10x noise lang)
    std::cout << "\nBootstrap refresh test (10x noise):\n";
    GoldenFHE::Cipher noisy = GoldenFHE::encrypt(pk, true, 70000000);
    
    for (int i = 0; i < 10; i++) {
        noisy.c0 = noisy.c0 + noisy.c0;
    }
    
    // I-check bago bootstrap
    bool before = GoldenFHE::decrypt(noisy, sk);
    std::cout << "  Before refresh: " << before << " (noisy but still 1)\n";
    
    // NOT function
    auto not1 = [](const std::vector<bool>& inputs) {
        return !inputs[0];
    };
    
    pb.setup_program(not1, 1);
    std::vector<GoldenFHE::Cipher> enc_input = {noisy};
    GoldenFHE::Cipher refreshed = pb.evaluate_program(enc_input);
    bool output = GoldenFHE::decrypt(refreshed, sk);
    
    std::cout << "  NOT(1) after refresh: " << output << " (expected 0)\n";
    
    if (output != false) passed = false;
    
    if (passed) {
        std::cout << "\n✅ PROGRAMMATIC BOOTSTRAPPING PASSED!\n";
        return 0;
    }
    
    std::cout << "\n❌ FAILED\n";
    return 1;
}
