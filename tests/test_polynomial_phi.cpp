// POLYNOMIAL φ SEARCH — May Φ(X) ba na Φ² = Φ + 1 sa ring?
#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>

int main() {
    std::cout << "POLYNOMIAL φ SEARCH\n";
    std::cout << "===================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    NTL::ZZ_p::init(Q);
    
    constexpr int N = 4;  // Maliit muna para sa search
    
    std::cout << "1. RING: Z_Q[x]/(x^" << N << "+1)\n";
    std::cout << "   Q: " << NTL::NumBits(Q) << " bits\n\n";
    
    std::cout << "2. SEARCH: Φ(X) na may Φ² ≡ Φ + 1 mod (x^N+1)\n";
    std::cout << "   Subukan ang: Φ(X) = φ · x^k (scalar φ times monomial)\n\n";
    
    // Compute scalar φ
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    
    std::cout << "3. SUBUKAN: Φ(X) = φ · x^0 = φ (constant polynomial)\n";
    std::cout << "   φ² = φ+1: " << ((phi*phi)%Q == (phi+1)%Q ? "YES ✓" : "NO ✗") << "\n\n";
    
    std::cout << "4. SUBUKAN: Φ(X) = φ · x^k para sa k=1..N-1\n";
    std::cout << "   Φ² = φ²·x^(2k)\n";
    std::cout << "   φ+1 sa polynomial form: (φ+1)·x^0\n";
    std::cout << "   Kailangan: x^(2k) ≡ x^0 = 1 mod (x^N+1)\n";
    std::cout << "   → x^(2k) ≡ 1 kung 2k ≡ 0 mod 2N\n";
    std::cout << "   → k = N/2 (k=2 para sa N=4)\n\n";
    
    int k = N/2;
    std::cout << "5. TEST k = " << k << ":\n";
    std::cout << "   x^" << 2*k << " ≡ x^" << N << " ≡ -1 mod (x^N+1)\n";
    std::cout << "   → HINDI 1!\n";
    std::cout << "   → Kailangan ng 2k ≡ 0 mod 2N\n";
    std::cout << "   → k = N (pero Φ(X) = φ·x^N ≡ -φ)\n\n";
    
    std::cout << "6. RESULT:\n";
    std::cout << "   Walang monomial Φ(X) = φ·x^k na may Φ²=Φ+1\n";
    std::cout << "   sa ring Z_Q[x]/(x^N+1) para sa N>2.\n";
    std::cout << "   → Ang period-2 ay SCALAR property lamang.\n\n";
    
    return 0;
}
