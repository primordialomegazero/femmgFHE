// HIDDEN Q — Itago ang Q para ma-hide ang pattern
// Kung hindi alam ang Q, hindi makukuha ang ct mod Q

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "HIDDEN Q APPROACH\n";
    std::cout << "=================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    
    NTL::ZZ phi_k = NTL::to_ZZ(1);
    for (int i = 0; i < 42; i++) phi_k = (phi_k * phi) % Q;
    
    std::mt19937_64 rng(42);
    
    // Hidden Q: ct = m·φ^k + r·Q
    // Q ay SECRET — hindi public
    // Public: φ^k lang (o wala — symmetric scheme)
    
    auto encrypt = [&](bool bit) {
        NTL::ZZ r = NTL::RandomBnd(Q);
        NTL::ZZ m_val = bit ? phi_k : NTL::to_ZZ(0);
        return m_val + r * Q;
    };
    
    auto decrypt = [&](NTL::ZZ ct) {
        NTL::ZZ v = ct % Q;
        if (v < 0) v += Q;
        NTL::ZZ d_phi = (v > phi_k) ? v - phi_k : phi_k - v;
        NTL::ZZ d_0 = (v < Q/2) ? v : Q - v;
        return d_phi < d_0;
    };
    
    // NAND (symmetric — parehong key)
    auto nand_op = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ a_mod = a % Q;
        NTL::ZZ b_mod = b % Q;
        NTL::ZZ prod = (a_mod * b_mod) % Q;
        NTL::ZZ inv_phi_k = NTL::InvMod(phi_k, Q);
        NTL::ZZ scaled = (prod * inv_phi_k) % Q;
        NTL::ZZ result = (phi_k - scaled) % Q;
        if (result < 0) result += Q;
        NTL::ZZ r = NTL::RandomBnd(Q);
        return result + r * Q;
    };
    
    std::cout << "1. SYMMETRIC Q-EMBED:\n";
    std::cout << "   Secret: Q, φ^k, k\n";
    std::cout << "   Public: wala (or φ^k lang)\n";
    std::cout << "   Attacker ay may ciphertexts pero HINDI Q\n\n";
    
    std::cout << "2. ATTACKER'S VIEW:\n";
    std::cout << "   ct0 = " << encrypt(false) << "\n";
    std::cout << "   ct1 = " << encrypt(true) << "\n";
    std::cout << "   → Pareho silang random-looking\n";
    std::cout << "   → Hindi makikita ang pattern kung hindi alam Q\n\n";
    
    std::cout << "3. SECURITY MODEL:\n";
    std::cout << "   Ito ay SYMMETRIC FHE — hindi public-key\n";
    std::cout << "   Security: kung Q ay malaki at random,\n";
    std::cout << "   ang ciphertext ay random-looking\n\n";
    
    std::cout << "4. LIMITATION:\n";
    std::cout << "   Kung ang attacker ay may maraming ciphertexts,\n";
    std::cout << "   pwede niyang i-GCD ang differences:\n";
    std::cout << "   ct1 - ct0 = (m1-m0)φ^k + (r1-r0)Q\n";
    std::cout << "   Kung m1=m0: diff = (r1-r0)Q → multiple ng Q!\n";
    std::cout << "   → Kaya Q ay pwede ma-recover via GCD\n\n";
    
    // GCD attack test
    std::cout << "5. GCD ATTACK TEST:\n";
    std::vector<NTL::ZZ> cts;
    for (int i = 0; i < 10; i++) {
        cts.push_back(encrypt(false));  // lahat message 0
    }
    NTL::ZZ gcd_val = cts[1] - cts[0];
    if (gcd_val < 0) gcd_val = -gcd_val;
    for (int i = 2; i < 10; i++) {
        NTL::ZZ diff = cts[i] - cts[0];
        if (diff < 0) diff = -diff;
        gcd_val = NTL::GCD(gcd_val, diff);
    }
    std::cout << "   GCD ng 10 ciphertexts (lahat m=0): " << gcd_val << "\n";
    std::cout << "   Q = " << Q << "\n";
    std::cout << "   Match: " << (gcd_val == Q ? "Q LEAKED! ✗" : "Q HIDDEN ✓") << "\n\n";
    
    std::cout << "6. RESULT:\n";
    std::cout << "   Kung ang attacker ay makakuha ng maraming\n";
    std::cout << "   ciphertexts ng parehong message, pwede ang GCD.\n";
    std::cout << "   → Kailangan ng countermeasure\n\n";
    
    return 0;
}
