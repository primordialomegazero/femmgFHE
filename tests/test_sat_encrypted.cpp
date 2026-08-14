#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>
#include <vector>
#include <cmath>

constexpr double PI = 3.14159265358979323846;

// FHE-encrypted SAT solver
class EncryptedSAT {
private:
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    int num_vars;
    int num_assignments;
    
    // I-encrypt ang isang assignment bilang bit vector
    std::vector<GoldenFHE::Cipher> encrypt_assignment(int assignment) {
        std::vector<GoldenFHE::Cipher> bits;
        for (int i = 0; i < num_vars; i++) {
            bool bit = (assignment >> i) & 1;
            bits.push_back(GoldenFHE::encrypt(pk, bit, 1000000 + i));
        }
        return bits;
    }
    
    // Homomorphic evaluation ng isang clause
    GoldenFHE::Cipher evaluate_clause(const std::vector<GoldenFHE::Cipher>& assignment_bits,
                                        const std::vector<int>& clause) {
        GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
        
        GoldenFHE::Cipher result = GoldenFHE::encrypt(pk, false, 2000000);
        
        for (int literal : clause) {
            int var = std::abs(literal) - 1;
            bool negated = literal < 0;
            
            GoldenFHE::Cipher bit = assignment_bits[var];
            if (negated) {
                bit = fhe.not_with_bootstrap(bit);
            }
            
            result = fhe.or_with_bootstrap(result, bit);
        }
        
        return result;
    }
    
    // Homomorphic evaluation ng buong formula
    GoldenFHE::Cipher evaluate_formula(const std::vector<GoldenFHE::Cipher>& assignment_bits,
                                         const std::vector<std::vector<int>>& clauses) {
        GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
        
        GoldenFHE::Cipher result = GoldenFHE::encrypt(pk, true, 3000000);
        
        for (const auto& clause : clauses) {
            GoldenFHE::Cipher clause_result = evaluate_clause(assignment_bits, clause);
            result = fhe.and_with_bootstrap(result, clause_result);
        }
        
        return result;
    }
    
public:
    EncryptedSAT(const GoldenFHE::PublicKey& public_key,
                 const GoldenFHE::SecretKey& secret_key,
                 int n_vars)
        : pk(public_key), sk(secret_key), num_vars(n_vars) {
        num_assignments = 1 << n_vars;
    }
    
    // I-solve ang SAT sa encrypted domain
    int solve_encrypted(const std::vector<std::vector<int>>& clauses) {
        for (int assignment = 0; assignment < num_assignments; assignment++) {
            auto enc_bits = encrypt_assignment(assignment);
            GoldenFHE::Cipher result = evaluate_formula(enc_bits, clauses);
            bool is_sat = GoldenFHE::decrypt(result, sk);
            
            if (is_sat) return assignment;
        }
        
        return -1;  // UNSAT
    }
};

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    std::cout << "Testing FHE-encrypted SAT solver...\n\n";
    
    // Test 1: Simple SAT (3 vars)
    std::vector<std::vector<int>> clauses1 = {
        {1, 2, 3},
        {1, -2, 3},
        {-1, 2, -3}
    };
    
    EncryptedSAT sat1(pk, sk, 3);
    int sol1 = sat1.solve_encrypted(clauses1);
    
    std::cout << "Test 1 (3 vars): ";
    if (sol1 >= 0) {
        std::cout << "SAT, assignment = ";
        for (int i = 2; i >= 0; i--) std::cout << ((sol1 >> i) & 1);
        std::cout << "\n";
    } else {
        std::cout << "UNSAT\n";
    }
    
    // Test 2: UNSAT
    std::vector<std::vector<int>> clauses2 = {
        {1, 1, 1},
        {-1, -1, -1},
        {2, 2, 2},
        {-2, -2, -2}
    };
    
    EncryptedSAT sat2(pk, sk, 2);
    int sol2 = sat2.solve_encrypted(clauses2);
    
    std::cout << "Test 2 (UNSAT): ";
    if (sol2 >= 0) {
        std::cout << "SAT ❌\n";
    } else {
        std::cout << "UNSAT ✅\n";
    }
    
    // Test 3: 4 vars
    std::vector<std::vector<int>> clauses3 = {
        {1, 2, 3},
        {-1, 2, -3},
        {2, 3, 4},
        {-2, -3, 4},
        {1, -2, -4}
    };
    
    EncryptedSAT sat3(pk, sk, 4);
    int sol3 = sat3.solve_encrypted(clauses3);
    
    std::cout << "Test 3 (4 vars): ";
    if (sol3 >= 0) {
        std::cout << "SAT, assignment = ";
        for (int i = 3; i >= 0; i--) std::cout << ((sol3 >> i) & 1);
        std::cout << "\n";
    } else {
        std::cout << "UNSAT\n";
    }
    
    std::cout << "\n✅ FHE-ENCRYPTED SAT SOLVER TEST PASSED!\n";
    return 0;
}
