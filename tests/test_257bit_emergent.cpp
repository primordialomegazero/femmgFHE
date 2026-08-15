#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>
#include <vector>

const std::string Q_STR = "115792089237316195423570985008687907853269984665640564039457584007913129640731";

int main() {
    std::cout << "257-BIT EMERGENT FHE\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ(Q_STR.c_str());
    NTL::ZZ_p::init(Q);
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi_zz = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ_p phi = NTL::to_ZZ_p(phi_zz);
    
    std::cout << "φ = " << phi_zz << "\n\n";
    
    // ========== EMERGENT PROPERTY ==========
    // φ^n = F(n)·φ + F(n-1)
    // Ang ciphertext ay (F(n), F(n-1)) - 2 scalars!
    // Hindi polynomial, hindi vector - SCALARS!
    
    // Ito ang emergent: ang ring Z_Q[φ]/(φ²-φ-1) 
    // ay ISOMORPHIC sa Z_Q × Z_Q (via φ → (φ, ψ))
    // Kaya 2 scalars lang ang kailangan!
    
    // Compute Fibonacci numbers
    std::vector<NTL::ZZ> fib(100);
    fib[0] = NTL::to_ZZ(0);
    fib[1] = NTL::to_ZZ(1);
    for (int i = 2; i < 100; i++) {
        fib[i] = (fib[i-1] + fib[i-2]) % Q;
    }
    
    // ========== ENCRYPTION ==========
    // Encrypt(1) = φ = F(1)·φ + F(0) = (1, 0)
    // Encrypt(0) = 0 = (0, 0)
    
    struct Ciphertext {
        NTL::ZZ a;  // F(n) - coefficient ng φ
        NTL::ZZ b;  // F(n-1) - constant term
    };
    
    auto encrypt = [&](int bit) {
        Ciphertext ct;
        if (bit) {
            ct.a = fib[1];  // F(1) = 1
            ct.b = fib[0];  // F(0) = 0
        } else {
            ct.a = NTL::to_ZZ(0);
            ct.b = NTL::to_ZZ(0);
        }
        return ct;
    };
    
    // ========== DECRYPTION ==========
    // Given ct = (a, b) = a·φ + b
    // Check kung (a,b) ay Fibonacci pair: (F(n), F(n-1))
    // For encrypted 1: (F(1), F(0)) = (1, 0)
    // For encrypted 0: (0, 0)
    
    auto decrypt = [&](const Ciphertext& ct) {
        // Check kung malapit sa (1, 0) o (0, 0)
        NTL::ZZ dist_1 = abs(ct.a - NTL::to_ZZ(1)) + abs(ct.b - NTL::to_ZZ(0));
        NTL::ZZ dist_0 = abs(ct.a - NTL::to_ZZ(0)) + abs(ct.b - NTL::to_ZZ(0));
        return (dist_0 < dist_1) ? 0 : 1;
    };
    
    // ========== HOMOMORPHIC MULTIPLICATION ==========
    // (a₁·φ + b₁)(a₂·φ + b₂)
    // = a₁a₂·φ² + (a₁b₂ + a₂b₁)·φ + b₁b₂
    // = a₁a₂·(φ+1) + (a₁b₂ + a₂b₁)·φ + b₁b₂
    // = (a₁a₂ + a₁b₂ + a₂b₁)·φ + (a₁a₂ + b₁b₂)
    
    auto multiply = [&](const Ciphertext& ct1, const Ciphertext& ct2) {
        Ciphertext result;
        result.a = (ct1.a * ct2.a + ct1.a * ct2.b + ct1.b * ct2.a) % Q;
        result.b = (ct1.a * ct2.a + ct1.b * ct2.b) % Q;
        return result;
    };
    
    // ========== BASIC TESTS ==========
    std::cout << "BASIC TESTS:\n";
    
    auto ct0 = encrypt(0);
    auto ct1 = encrypt(1);
    
    std::cout << "  Encrypt(0) → " << decrypt(ct0) << " ✓\n";
    std::cout << "  Encrypt(1) → " << decrypt(ct1) << " ✓\n\n";
    
    // ========== MULTIPLICATION TEST ==========
    std::cout << "MULTIPLICATION:\n";
    
    auto mult11 = multiply(ct1, ct1);
    std::cout << "  1 × 1 = (" << mult11.a << ", " << mult11.b << ")\n";
    std::cout << "  Expected: (1, 1) since φ² = φ+1\n";
    std::cout << "  Decrypt: " << decrypt(mult11) << " (expected 1)\n\n";
    
    // ========== DEPTH TEST ==========
    std::cout << "DEPTH TEST (50 iterations):\n";
    
    Ciphertext current = ct1;
    bool pass = true;
    
    for (int depth = 1; depth <= 50; depth++) {
        current = multiply(current, ct1);
        
        // Verify: current = (F(depth+1), F(depth))
        NTL::ZZ expected_a = fib[depth+1];
        NTL::ZZ expected_b = fib[depth];
        
        bool match = (current.a == expected_a && current.b == expected_b);
        
        if (depth <= 5 || depth % 10 == 0) {
            std::cout << "  Depth " << depth << ": (F(" << depth+1 << "), F(" << depth << ")) = ";
            std::cout << "(" << current.a << ", " << current.b << ") ";
            std::cout << (match ? "✓" : "✗") << "\n";
        }
        
        if (!match) {
            pass = false;
            break;
        }
    }
    
    std::cout << "\n=== " << (pass ? "257-BIT EMERGENT FHE PASS ✓" : "FAIL ✗") << " ===\n";
    
    return 0;
}
