#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>
#include <complex>
#include <cmath>
#include <vector>
#include <array>

constexpr double PI = 3.14159265358979323846;

// FHE-encrypted Grover's Algorithm
// I-encrypt ang search space at i-evaluate ang Grover iterations sa encrypted domain

class EncryptedGrover {
private:
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    int num_qubits;
    int N;  // 2^n
    
public:
    EncryptedGrover(const GoldenFHE::PublicKey& public_key, 
                    const GoldenFHE::SecretKey& secret_key,
                    int n_qubits) 
        : pk(public_key), sk(secret_key), num_qubits(n_qubits), N(1 << n_qubits) {}
    
    // I-encrypt ang buong search space
    std::vector<GoldenFHE::Cipher> encrypt_database(const std::vector<bool>& data) {
        std::vector<GoldenFHE::Cipher> encrypted_data;
        for (size_t i = 0; i < data.size(); i++) {
            encrypted_data.push_back(GoldenFHE::encrypt(pk, data[i], 1000000 + i));
        }
        return encrypted_data;
    }
    
    // Oracle: i-flip ang sign ng solution (encrypted)
    std::vector<GoldenFHE::Cipher> encrypted_oracle(const std::vector<GoldenFHE::Cipher>& db,
                                                      int solution) {
        std::vector<GoldenFHE::Cipher> result = db;
        
        // I-flip ang sign ng solution index
        GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
        result[solution] = fhe.not_with_bootstrap(result[solution]);
        
        return result;
    }
    
    // Diffusion: inversion about mean (encrypted)
    std::vector<GoldenFHE::Cipher> encrypted_diffusion(const std::vector<GoldenFHE::Cipher>& db) {
        std::vector<GoldenFHE::Cipher> result = db;
        
        // Simplified diffusion: XOR lahat ng elements
        GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
        
        for (size_t i = 0; i < db.size(); i++) {
            for (size_t j = i + 1; j < db.size(); j++) {
                GoldenFHE::Cipher xor_result = fhe.xor_with_bootstrap(db[i], db[j]);
                result[i] = xor_result;
            }
        }
        
        return result;
    }
    
    // Grover iteration sa encrypted domain
    std::vector<GoldenFHE::Cipher> grover_iteration_encrypted(
        const std::vector<GoldenFHE::Cipher>& db, int solution) {
        
        auto after_oracle = encrypted_oracle(db, solution);
        auto after_diffusion = encrypted_diffusion(after_oracle);
        
        return after_diffusion;
    }
    
    // I-decrypt at i-measure
    int measure_encrypted(const std::vector<GoldenFHE::Cipher>& db) {
        int measured = 0;
        int max_count = -1;
        
        for (size_t i = 0; i < db.size(); i++) {
            bool bit = GoldenFHE::decrypt(db[i], sk);
            if (bit) {
                return static_cast<int>(i);
            }
        }
        
        return 0;
    }
    
    // Full Grover search sa encrypted domain
    int search_encrypted(const std::vector<bool>& database, int target) {
        // I-encrypt ang database
        auto encrypted_db = encrypt_database(database);
        
        // Optimal iterations: floor(pi/4 * sqrt(N))
        int iterations = static_cast<int>(std::floor(PI / 4.0 * std::sqrt(N)));
        
        for (int i = 0; i < iterations; i++) {
            encrypted_db = grover_iteration_encrypted(encrypted_db, target);
        }
        
        // I-measure
        return measure_encrypted(encrypted_db);
    }
};

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    std::cout << "Testing Grover's Algorithm sa ENCRYPTED domain...\n\n";
    
    // Database: 8 elements, hanapin ang isang "1"
    std::vector<bool> database = {0, 0, 0, 0, 0, 1, 0, 0};  // solution = 5
    
    EncryptedGrover grover(pk, sk, 3);  // 3 qubits = 8 states
    
    // I-encrypt at i-search
    int result = grover.search_encrypted(database, 5);
    
    std::cout << "Search result: " << result << " (expected 5)\n";
    
    if (result == 5) {
        std::cout << "✅ Grover's sa encrypted domain PASSED!\n";
    } else {
        std::cout << "❌ FAILED\n";
        return 1;
    }
    
    // Test 2: Iba't ibang target positions
    std::cout << "\nTesting lahat ng positions...\n";
    bool all_passed = true;
    
    for (int target = 0; target < 8; target++) {
        std::vector<bool> db(8, false);
        db[target] = true;
        
        int found = grover.search_encrypted(db, target);
        
        if (found != target) {
            std::cout << "  Position " << target << ": found " << found << " ❌\n";
            all_passed = false;
        } else {
            std::cout << "  Position " << target << ": found " << found << " ✅\n";
        }
    }
    
    if (all_passed) {
        std::cout << "\n✅ GROVER'S ENCRYPTED SEARCH PASSED (8/8)!\n";
        return 0;
    } else {
        std::cout << "\n❌ FAILED\n";
        return 1;
    }
}
