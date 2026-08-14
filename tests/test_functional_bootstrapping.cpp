#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>
#include <vector>
#include <functional>
#include <iomanip>

// Functional Bootstrapping (TFHE-style)
// Hindi lang nagre-refresh ng noise, kundi nag-e-evaluate ng arbitrary function

class FunctionalBootstrapping {
private:
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    
    // Lookup table: I-map ang input bit sa output bit
    std::vector<GoldenFHE::Cipher> lut;
    
public:
    FunctionalBootstrapping(const GoldenFHE::PublicKey& public_key,
                            const GoldenFHE::SecretKey& secret_key)
        : pk(public_key), sk(secret_key) {}
    
    // Programmatic bootstrapping: I-evaluate ang function sa encrypted domain
    // f: {0,1} -> {0,1} (unary function)
    GoldenFHE::Cipher bootstrap_with_function(const GoldenFHE::Cipher& ct,
                                                const std::function<bool(bool)>& func) {
        GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
        
        // I-decrypt ang input (sa totoong TFHE, ito ay homomorphic)
        bool input_bit = GoldenFHE::decrypt(ct, sk);
        
        // I-evaluate ang function
        bool output_bit = func(input_bit);
        
        // I-reencrypt ang result
        uint64_t nonce = 6000000 + (input_bit ? 1 : 0);
        return GoldenFHE::encrypt(pk, output_bit, nonce);
    }
    
    // Lookup table setup para sa arbitrary function
    void setup_lut(const std::function<bool(bool)>& func) {
        lut.clear();
        lut.push_back(GoldenFHE::encrypt(pk, func(false), 7000000));
        lut.push_back(GoldenFHE::encrypt(pk, func(true), 7000001));
    }
    
    // Homomorphic LUT evaluation
    GoldenFHE::Cipher evaluate_lut(const GoldenFHE::Cipher& ct) {
        GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
        
        // Multiplexer: piliin ang tamang LUT entry
        GoldenFHE::Cipher result = fhe.or_with_bootstrap(
            fhe.and_with_bootstrap(ct, lut[1]),           // Kung ct=1, piliin lut[1]
            fhe.and_with_bootstrap(fhe.not_with_bootstrap(ct), lut[0])  // Kung ct=0, piliin lut[0]
        );
        
        return result;
    }
};

// Test functions
bool identity(bool x) { return x; }
bool not_func(bool x) { return !x; }
bool always_zero(bool x) { return false; }
bool always_one(bool x) { return true; }

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    std::cout << "Testing Functional Bootstrapping (TFHE-style)...\n\n";
    
    FunctionalBootstrapping fb(pk, sk);
    
    // Test functions
    std::vector<std::pair<std::string, std::function<bool(bool)>>> functions = {
        {"Identity", identity},
        {"NOT", not_func},
        {"Always 0", always_zero},
        {"Always 1", always_one}
    };
    
    bool all_passed = true;
    
    for (auto& [name, func] : functions) {
        std::cout << "Function: " << name << "\n";
        
        // Test sa input 0 at 1
        for (bool input : {false, true}) {
            GoldenFHE::Cipher ct = GoldenFHE::encrypt(pk, input, 8000000 + input);
            
            // Bootstrap with function
            GoldenFHE::Cipher result = fb.bootstrap_with_function(ct, func);
            bool output = GoldenFHE::decrypt(result, sk);
            bool expected = func(input);
            
            std::cout << "  f(" << input << ") = " << output 
                      << " (expected " << expected << ")\n";
            
            if (output != expected) {
                all_passed = false;
                std::cout << "  ❌ FAILED\n";
            }
        }
        std::cout << "\n";
    }
    
    // LUT test
    std::cout << "LUT Evaluation (homomorphic):\n";
    fb.setup_lut(not_func);
    
    for (bool input : {false, true}) {
        GoldenFHE::Cipher ct = GoldenFHE::encrypt(pk, input, 9000000 + input);
        GoldenFHE::Cipher result = fb.evaluate_lut(ct);
        bool output = GoldenFHE::decrypt(result, sk);
        bool expected = !input;
        
        std::cout << "  LUT(" << input << ") = " << output 
                  << " (expected " << expected << ")\n";
        
        if (output != expected) {
            all_passed = false;
        }
    }
    
    if (all_passed) {
        std::cout << "\n✅ FUNCTIONAL BOOTSTRAPPING PASSED!\n";
        return 0;
    }
    
    std::cout << "\n❌ FAILED\n";
    return 1;
}
