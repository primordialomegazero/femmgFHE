// Q-EMBED ATTACK TEST — GCD attack para mahanap ang Q
#include "../src/fhe/golden_fhe_qembed.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "Q-EMBED ATTACK ANALYSIS\n";
    std::cout << "======================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    golden_fhe_qembed::QEmbedFHE fhe(Q, 42);
    
    // Generate ciphertexts
    std::vector<NTL::ZZ> cts;
    for (int i = 0; i < 10; i++) {
        cts.push_back(fhe.encrypt(i % 2 == 0));
    }
    
    std::cout << "1. CIPHERTEXT DIFFERENCES:\n";
    for (int i = 1; i < 10; i++) {
        NTL::ZZ diff = cts[i] - cts[0];
        if (diff < 0) diff = -diff;
        std::cout << "  |ct" << i << " - ct0| = " << diff << "\n";
    }
    std::cout << "\n";
    
    std::cout << "2. GCD ATTACK:\n";
    std::cout << "   Kung ang differences ay multiples ng Q,\n";
    std::cout << "   ang GCD ng differences ay nagbibigay ng Q!\n\n";
    
    NTL::ZZ gcd_val = cts[1] - cts[0];
    if (gcd_val < 0) gcd_val = -gcd_val;
    
    for (int i = 2; i < 10; i++) {
        NTL::ZZ diff = cts[i] - cts[0];
        if (diff < 0) diff = -diff;
        gcd_val = NTL::GCD(gcd_val, diff);
    }
    
    std::cout << "  GCD ng differences: " << gcd_val << "\n";
    std::cout << "  Actual Q: " << Q << "\n";
    std::cout << "  Match: " << (gcd_val == Q ? "YES — Q LEAKED! ✗" : "NO — Q hidden ✓") << "\n\n";
    
    std::cout << "3. RESULT:\n";
    if (gcd_val == Q || gcd_val % Q == 0) {
        std::cout << "  ❌ GCD attack ay SUCCESSFUL!\n";
        std::cout << "  Ang attacker ay makukuha ang Q mula sa differences.\n";
        std::cout << "  Pagkatapos: ct mod Q = m·φ^k → message revealed.\n";
        std::cout << "  → HINDI SECURE!\n";
    } else {
        std::cout << "  ✓ GCD attack ay hindi direct na nagre-reveal ng Q.\n";
        std::cout << "  Pero kailangan pa ng mas malalim na analysis.\n";
    }
    
    return 0;
}
