#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>
#include <vector>
#include <functional>
#include <chrono>

class ArbitraryFHE {
private:
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    
    std::vector<GoldenFHE::Cipher> encrypt_truth_table(
        const std::function<bool(const std::vector<bool>&)>& func,
        int num_inputs) {
        
        int num_combos = 1 << num_inputs;
        std::vector<GoldenFHE::Cipher> table;
        
        for (int i = 0; i < num_combos; i++) {
            std::vector<bool> inputs(num_inputs);
            for (int j = 0; j < num_inputs; j++) {
                inputs[j] = (i >> j) & 1;
            }
            
            bool output = func(inputs);
            table.push_back(GoldenFHE::encrypt(pk, output, 1000000 + i));
        }
        
        return table;
    }
    
public:
    ArbitraryFHE(const GoldenFHE::PublicKey& public_key,
                 const GoldenFHE::SecretKey& secret_key)
        : pk(public_key), sk(secret_key) {}
    
    bool evaluate_encrypted(
        const std::function<bool(const std::vector<bool>&)>& func,
        const std::vector<bool>& inputs) {
        
        int num_inputs = inputs.size();
        auto table = encrypt_truth_table(func, num_inputs);
        
        std::vector<GoldenFHE::Cipher> enc_inputs;
        for (int i = 0; i < num_inputs; i++) {
            enc_inputs.push_back(GoldenFHE::encrypt(pk, inputs[i], 2000000 + i));
        }
        
        GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
        GoldenFHE::Cipher result = table[0];
        
        for (int i = 0; i < static_cast<int>(table.size()); i++) {
            GoldenFHE::Cipher match = GoldenFHE::encrypt(pk, true, 3000000 + i);
            
            for (int j = 0; j < num_inputs; j++) {
                bool expected = (i >> j) & 1;
                
                GoldenFHE::Cipher expected_ct = GoldenFHE::encrypt(pk, expected, 4000000 + i * 10 + j);
                GoldenFHE::Cipher input_eq = fhe.xor_with_bootstrap(enc_inputs[j], expected_ct);
                GoldenFHE::Cipher not_equal = fhe.not_with_bootstrap(input_eq);
                
                match = fhe.and_with_bootstrap(match, not_equal);
            }
            
            GoldenFHE::Cipher selected = fhe.and_with_bootstrap(match, table[i]);
            result = fhe.or_with_bootstrap(result, selected);
        }
        
        return GoldenFHE::decrypt(result, sk);
    }
};

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    std::cout << "Testing Arbitrary Function Evaluation sa FHE...\n\n";
    
    ArbitraryFHE arb_fhe(pk, sk);
    
    // Test 1: XOR
    auto xor_func = [](const std::vector<bool>& inputs) {
        return inputs[0] ^ inputs[1];
    };
    
    std::cout << "XOR function:\n";
    bool xor_cases[][2] = {{0, 0}, {0, 1}, {1, 0}, {1, 1}};
    bool xor_passed = true;
    
    for (auto& tc : xor_cases) {
        bool result = arb_fhe.evaluate_encrypted(xor_func, {tc[0], tc[1]});
        bool expected = tc[0] ^ tc[1];
        
        std::cout << "  XOR(" << tc[0] << "," << tc[1] << ") = " 
                  << result << " (expected " << expected << ")\n";
        
        if (result != expected) xor_passed = false;
    }
    
    if (!xor_passed) {
        std::cout << "  ❌ XOR FAILED\n";
        return 1;
    }
    
    // Test 2: Majority
    auto majority = [](const std::vector<bool>& inputs) {
        int count = inputs[0] + inputs[1] + inputs[2];
        return count >= 2;
    };
    
    std::cout << "\nMajority function (3 inputs):\n";
    bool maj_passed = true;
    
    for (int i = 0; i < 8; i++) {
        std::vector<bool> inputs = {
            (bool)((i >> 0) & 1),
            (bool)((i >> 1) & 1),
            (bool)((i >> 2) & 1)
        };
        
        bool result = arb_fhe.evaluate_encrypted(majority, inputs);
        bool expected = (i == 3 || i == 5 || i == 6 || i == 7);
        
        std::cout << "  Maj(" << inputs[0] << "," << inputs[1] << "," << inputs[2] 
                  << ") = " << result << " (expected " << expected << ")\n";
        
        if (result != expected) maj_passed = false;
    }
    
    if (!maj_passed) {
        std::cout << "  ❌ Majority FAILED\n";
        return 1;
    }
    
    // Test 3: Parity
    auto parity = [](const std::vector<bool>& inputs) {
        bool result = false;
        for (bool b : inputs) result ^= b;
        return result;
    };
    
    std::cout << "\nParity function (3 inputs):\n";
    bool par_passed = true;
    
    for (int i = 0; i < 8; i++) {
        std::vector<bool> inputs = {
            (bool)((i >> 0) & 1),
            (bool)((i >> 1) & 1),
            (bool)((i >> 2) & 1)
        };
        
        bool result = arb_fhe.evaluate_encrypted(parity, inputs);
        bool expected = (i == 1 || i == 2 || i == 4 || i == 7);
        
        std::cout << "  Parity(" << inputs[0] << "," << inputs[1] << "," << inputs[2] 
                  << ") = " << result << " (expected " << expected << ")\n";
        
        if (result != expected) par_passed = false;
    }
    
    if (!par_passed) {
        std::cout << "  ❌ Parity FAILED\n";
        return 1;
    }
    
    std::cout << "\n✅ ARBITRARY FUNCTION EVALUATION PASSED!\n";
    return 0;
}
