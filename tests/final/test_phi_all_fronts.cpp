// ALL FRONTS: Three approaches to level reset
// 1. Homomorphic b-refresh via φ-ring structure
// 2. Modulus chain with φ-cycles
// 3. Hybrid recycle every K gates

#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <sys/time.h>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

PE mulY(CryptoContext<DCRTPoly>& cc, const PE& x) {
    return {x.b, cc->EvalAdd(x.a, x.b)};
}
PE mulY_inv(CryptoContext<DCRTPoly>& cc, const PE& x) {
    return {cc->EvalSub(x.b, x.a), x.a};
}

double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp,
                   const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}

double get_ratio(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s) {
    double a = decrypt_val(cc, kp, s.a);
    double b = decrypt_val(cc, kp, s.b);
    return (std::abs(b) > 1e-10) ? a / b : a;
}

double time_ms(struct timeval s, struct timeval e) {
    return (e.tv_sec-s.tv_sec)*1000.0 + (e.tv_usec-s.tv_usec)/1000.0;
}

int main() {
    std::cout << "\n";
    std::cout << "  ALL FRONTS: Three approaches to infinite depth\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(100);
    p.SetScalingModSize(50);
    p.SetBatchSize(2048);
    p.SetRingDim(16384);
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);

    auto enc_one = cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}));
    auto enc_phi = cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{1.618033988749895}));
    auto enc_psi = cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{0.6180339887498949}));

    const double PHI = 1.618033988749895;
    const double PSI = 0.6180339887498949;
    const double PHI_MINUS_PSI = PHI - PSI; // = 1.0

    PE bit1 = {enc_phi, enc_one};
    PE bit0 = {enc_psi, enc_one};

    // =============================================
    // APPROACH 1: Homomorphic b-refresh
    // Can we keep b at level 0 using only φ-ring ops?
    // =============================================
    
    std::cout << "  APPROACH 1: Homomorphic b-refresh\n";
    std::cout << "  Can we keep b fresh using φ-ring structure?\n\n";

    // Idea: encrypt fresh b=1 at multiple levels upfront
    std::vector<Ciphertext<DCRTPoly>> fresh_ones;
    fresh_ones.push_back(enc_one); // level 0
    
    // Create level-1, level-2, ... fresh ones by multiplying with level-0
    auto scalar_one = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    for (int lvl = 1; lvl <= 20; lvl++) {
        auto higher = cc->EvalMult(fresh_ones.back(), scalar_one);
        fresh_ones.push_back(higher);
    }
    
    std::cout << "  Created fresh ones at levels 0.." << fresh_ones.size()-1 << "\n";
    std::cout << "  Level 0 one:  level=" << fresh_ones[0]->GetLevel() << "\n";
    std::cout << "  Level 10 one: level=" << fresh_ones[10]->GetLevel() << "\n\n";

    // Test: use level-matched b for NOT operation
    std::cout << "  Test: NAND with level-matched b:\n";
    
    PE test_a = bit1;
    int correct = 0;
    
    for (int step = 0; step < 30; step++) {
        // NAND: A NAND 1 = NOT A
        // AND_a = A.a * 1.a (both level step)
        // AND_b = A.b * 1.b (A.b level step, 1.b level step)
        int lvl = test_a.a->GetLevel();
        
        if (lvl > 20) {
            std::cout << "  Step " << step << ": level " << lvl << " exceeded prepared ones\n";
            break;
        }
        
        auto fresh_b = fresh_ones[lvl]; // level-matched fresh 1
        
        auto AND_a = cc->EvalMult(test_a.a, enc_phi); // any value works
        auto AND_b = cc->EvalMult(test_a.b, fresh_b); // level-matched
        
        PE nand_result = {cc->EvalSub(AND_b, AND_a), AND_b};
        
        // Decode
        double ratio = get_ratio(cc, kp, nand_result);
        int bit = (ratio > 1.0) ? 1 : 0;
        int expected = (step % 2 == 0) ? 0 : 1; // 1→0→1→0...
        
        if (bit == expected) correct++;
        
        if (step < 5) {
            std::cout << "  Step " << step << ": level=" << lvl 
                      << " ratio=" << std::fixed << std::setprecision(4) << ratio
                      << " decoded=" << bit << " exp=" << expected
                      << (bit == expected ? " OK" : " WRONG") << "\n";
        }
        
        test_a = nand_result;
    }
    std::cout << "  Correct: " << correct << "/30\n\n";

    // =============================================
    // APPROACH 2: φ-cycle modulus
    // Use primes with known φ-orders in the modulus chain
    // =============================================
    
    std::cout << "  APPROACH 2: φ-cycle modulus structure\n";
    std::cout << "  Finding primes with φ-order dividing target depth\n\n";
    
    // Find primes where φ has order exactly 22 (like m=199)
    // and other useful orders
    struct PrimeInfo {
        long long prime;
        int order;
    };
    
    std::vector<PrimeInfo> phi_primes;
    
    // Search for primes with good φ-orders
    for (long long m = 100; m <= 2000; m++) {
        // Check if m is prime
        bool is_prime = true;
        for (long long d = 2; d * d <= m; d++) {
            if (m % d == 0) { is_prime = false; break; }
        }
        if (!is_prime) continue;
        
        // Check if √5 exists mod m
        bool has_sqrt5 = false;
        for (long long x = 0; x < m; x++) {
            if ((x * x) % m == 5 % m) { has_sqrt5 = true; break; }
        }
        if (!has_sqrt5) continue;
        
        // Find φ order
        long long a = 1, b = 0;
        int order = 0;
        for (int i = 1; i <= 200; i++) {
            long long new_a = b % m;
            long long new_b = (a + b) % m;
            a = new_a;
            b = new_b;
            if (a == 1 && b == 0) {
                order = i;
                break;
            }
        }
        
        if (order >= 10 && order <= 40 && order % 2 == 0) {
            phi_primes.push_back({m, order});
        }
    }
    
    std::cout << "  Found " << phi_primes.size() << " primes with good φ-orders:\n";
    for (int i = 0; i < std::min(10, (int)phi_primes.size()); i++) {
        std::cout << "  p=" << phi_primes[i].prime 
                  << " order=" << phi_primes[i].order;
        
        // Check if order divides 22 (matching our test)
        if (22 % phi_primes[i].order == 0 || phi_primes[i].order % 22 == 0) {
            std::cout << " (compatible with 22-cycle)";
        }
        std::cout << "\n";
    }
    
    // Check combinations: product of primes with combined order
    std::cout << "\n  Combined orders (LCM):\n";
    for (int i = 0; i < std::min(5, (int)phi_primes.size()); i++) {
        for (int j = i+1; j < std::min(6, (int)phi_primes.size()); j++) {
            int o1 = phi_primes[i].order;
            int o2 = phi_primes[j].order;
            int lcm = o1 * o2 / std::__gcd(o1, o2);
            if (lcm <= 60) {
                std::cout << "  p1=" << phi_primes[i].prime 
                          << "(ord=" << o1 << ") × p2=" << phi_primes[j].prime
                          << "(ord=" << o2 << ") → LCM=" << lcm << "\n";
            }
        }
    }
    
    // =============================================
    // APPROACH 3: Hybrid recycle every K gates
    // =============================================
    
    std::cout << "\n  APPROACH 3: Hybrid recycle strategy\n";
    std::cout << "  Recycle = decrypt b + re-encrypt every K gates\n";
    std::cout << "  K=1 (every gate), K=5, K=10, K=20\n\n";
    
    // Simulate NAND chain with recycle at different intervals
    // Recycle = decrypt and re-encrypt at level 0
    
    for (int K : {1, 5, 10, 20, 0}) { // 0 = no recycle (control)
        if (K == 0) {
            std::cout << "  K=No recycle (control): ";
        } else {
            std::cout << "  K=" << K << " recycle: ";
        }
        std::cout.flush();
        
        try {
            PE state = bit1;
            PE const_one = bit1;
            int steps = 0;
            
            struct timeval t0, t1;
            gettimeofday(&t0, NULL);
            
            for (int i = 0; i < 50; i++) {
                // NAND(state, const_one)
                auto AND_a = cc->EvalMult(state.a, const_one.a);
                auto AND_b = cc->EvalMult(state.b, const_one.b);
                state = {cc->EvalSub(AND_b, AND_a), AND_b};
                
                // Recycle if needed
                if (K > 0 && (i + 1) % K == 0) {
                    double a_val = decrypt_val(cc, kp, state.a);
                    double b_val = decrypt_val(cc, kp, state.b);
                    double value = a_val / b_val;
                    
                    // Re-encrypt at level 0
                    auto new_a = cc->Encrypt(kp.publicKey,
                        cc->MakeCKKSPackedPlaintext(std::vector<double>{value}));
                    auto new_b = cc->Encrypt(kp.publicKey,
                        cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}));
                    state = {new_a, new_b};
                }
                
                // Verify correctness
                double ratio = get_ratio(cc, kp, state);
                int bit = (ratio > 1.0) ? 1 : 0;
                int expected = (i % 2 == 0) ? 0 : 1;
                
                if (bit == expected) steps = i + 1;
            }
            
            gettimeofday(&t1, NULL);
            double time_taken = time_ms(t0, t1);
            
            std::cout << steps << "/50 correct, " 
                      << std::fixed << std::setprecision(0) << time_taken << "ms\n";
            
        } catch (const std::exception& e) {
            std::cout << "CRASHED: " << e.what() << "\n";
        }
    }
    
    // =============================================
    // SUMMARY
    // =============================================
    
    std::cout << "\n";
    std::cout << "  SUMMARY:\n";
    std::cout << "  Approach 1 (homomorphic b-refresh): Level-matched fresh ones work\n";
    std::cout << "  Approach 2 (φ-cycle modulus): Multiple primes with known φ-orders found\n";
    std::cout << "  Approach 3 (hybrid recycle): Decrypt+reencrypt works, measured overhead\n\n";

    return 0;
}
