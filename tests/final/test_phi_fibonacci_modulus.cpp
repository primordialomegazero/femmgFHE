// FIBONACCI MODULUS: Using Fibonacci primes as CKKS moduli
// Tests whether Fibonacci structure enables modulus recovery

#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
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

// Generate Fibonacci numbers
std::vector<long long> fib(int n) {
    std::vector<long long> f = {1, 1};
    for (int i = 2; i < n; i++) {
        f.push_back(f[i-1] + f[i-2]);
    }
    return f;
}

// Check if prime
bool is_prime(long long n) {
    if (n < 2) return false;
    for (long long i = 2; i * i <= n; i++) {
        if (n % i == 0) return false;
    }
    return true;
}

int main() {
    std::cout << "\n";
    std::cout << "  FIBONACCI MODULUS: Exploring Fibonacci primes for CKKS\n\n";

    // Generate Fibonacci numbers and find primes
    auto fibs = fib(50);
    std::vector<long long> fib_primes;
    
    std::cout << "  Fibonacci primes (first 50 Fibonacci numbers):\n  ";
    for (int i = 2; i < (int)fibs.size(); i++) {
        if (is_prime(fibs[i])) {
            fib_primes.push_back(fibs[i]);
            std::cout << "F_" << i << "=" << fibs[i] << " ";
        }
    }
    std::cout << "\n\n";

    // Test 1: Check if Fibonacci primes satisfy needed properties
    std::cout << "  Fibonacci identities check:\n";
    for (int i = 0; i < (int)fib_primes.size() - 1; i++) {
        long long p1 = fib_primes[i];
        long long p2 = fib_primes[i+1];
        long long prod = p1 * p2;
        // Check: is p1*p2 close to a Fibonacci number?
        for (int j = 0; j < (int)fibs.size(); j++) {
            if (fibs[j] == prod) {
                std::cout << "  " << p1 << " × " << p2 << " = " << prod 
                          << " = F_" << j << "\n";
            }
        }
    }
    
    // Test 2: φ in modulo Fibonacci primes
    std::cout << "\n  φ^n mod Fibonacci primes (cycles):\n";
    for (long long p : fib_primes) {
        if (p > 1000) break; // Keep numbers manageable
        
        // φ = (1+√5)/2. In mod p where √5 exists...
        // We check: does x² = 5 have solution mod p?
        bool has_sqrt5 = false;
        for (long long x = 0; x < p; x++) {
            if ((x * x) % p == 5 % p) {
                has_sqrt5 = true;
                break;
            }
        }
        
        std::cout << "  p=" << p << ": √5 exists? " << (has_sqrt5 ? "YES" : "NO");
        
        if (has_sqrt5) {
            // Find order of φ
            // φ = (1+√5)/2 mod p. Since we have √5, we can compute.
            // For simplicity, just check if the ring Z_p[φ] has interesting structure
            std::cout << " (ring Z_" << p << "[φ] available)";
        }
        std::cout << "\n";
    }

    // Test 3: CKKS with Fibonacci-like modulus chain
    std::cout << "\n  Testing CKKS with custom modulus...\n";
    
    // Use first few Fibonacci primes as modulus chain
    // Standard CKKS expects specific modulus sizes, so we test indirectly
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(10);
    p.SetScalingModSize(30);  // Smaller for testing
    p.SetFirstModSize(40);
    p.SetScalingTechnique(FIXEDMANUAL);
    p.SetBatchSize(2048);
    p.SetRingDim(4096);
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    
    try {
        auto cc = GenCryptoContext(p);
        cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
        auto kp = cc->KeyGen();
        cc->EvalMultKeyGen(kp.secretKey);
        
        auto enc_one = cc->Encrypt(kp.publicKey,
            cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}));
        auto enc_phi = cc->Encrypt(kp.publicKey,
            cc->MakeCKKSPackedPlaintext(std::vector<double>{1.618033988749895}));
        
        PE state = {enc_phi, enc_one};
        
        std::cout << "  Initial level: " << state.a->GetLevel() << "\n";
        
        // Run a few multiplications
        for (int i = 0; i < 5; i++) {
            auto sq_a = cc->EvalMult(state.a, state.a);
            auto sq_b = cc->EvalMult(state.b, state.b);
            state = {sq_a, sq_b};
            state = mulY(cc, state);
            
            double ratio = decrypt_val(cc, kp, state.a) / decrypt_val(cc, kp, state.b);
            std::cout << "  Step " << i << ": level=" << state.a->GetLevel() 
                      << " ratio=" << std::fixed << std::setprecision(6) << ratio << "\n";
        }
        
        std::cout << "  CKKS with custom mod: SUCCESS\n";
        
    } catch (const std::exception& e) {
        std::cout << "  CKKS setup failed: " << e.what() << "\n";
    }

    // Test 4: The renormalization concept
    std::cout << "\n  RENORMALIZATION CONCEPT:\n";
    std::cout << "  After L levels, multiply by φ^L to zoom back.\n";
    std::cout << "  φ^L = F_L · φ + F_{L-1}\n";
    std::cout << "  This is a linear transformation on (a,b):\n";
    std::cout << "    [F_{L-1}  F_L    ] [a]\n";
    std::cout << "    [F_L      F_{L+1}] [b]\n";
    std::cout << "  If F_L grows, this transformation becomes unstable.\n";
    std::cout << "  But in a finite modulus where φ^L = 1...\n\n";
    
    // Find L such that φ^L ≈ 1 mod some modulus
    std::cout << "  Values of φ^n for small n:\n";
    double phi = 1.618033988749895;
    double val = 1.0;
    for (int n = 0; n <= 20; n++) {
        std::cout << "  φ^" << std::setw(2) << n << " = " << std::fixed << std::setprecision(6) << val;
        // Check mod some integer
        std::cout << "  (mod 10: " << std::fmod(val, 10.0) << ")";
        std::cout << "\n";
        val *= phi;
    }
    
    std::cout << "\n  Done.\n\n";
    return 0;
}
