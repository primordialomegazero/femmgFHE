// DEEPER ATTACK: Statistical analysis ng ciphertexts
#include "../src/fhe/golden_fhe_qembed.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "DEEPER ATTACK ANALYSIS\n";
    std::cout << "=====================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    golden_fhe_qembed::QEmbedFHE fhe(Q, 42);
    
    // Generate 1000 ciphertexts
    std::vector<NTL::ZZ> cts;
    for (int i = 0; i < 1000; i++) {
        cts.push_back(fhe.encrypt(i % 2 == 0));
    }
    
    std::cout << "1. DISTRIBUTION CHECK:\n";
    std::cout << "   ct mod Q ay dapat 0 o φ^k\n";
    std::cout << "   Kung may pattern, attacker ay makaka-distinguish\n\n";
    
    // Check ct mod Q values
    int zeros = 0, ones = 0;
    std::cout << "   First 10 ct mod Q values:\n";
    for (int i = 0; i < 10; i++) {
        NTL::ZZ v = cts[i] % Q;
        if (v < 0) v += Q;
        if (v == 0) {
            zeros++;
            std::cout << "   ct" << i << " mod Q = 0 (message 0)\n";
        } else {
            ones++;
            std::cout << "   ct" << i << " mod Q = " << v << " (message 1)\n";
        }
    }
    std::cout << "\n";
    
    std::cout << "2. ATTACK SCENARIO:\n";
    std::cout << "   Attacker ay nakakakita ng ct0, ct1, ct2, ...\n";
    std::cout << "   Hindi alam ang Q, φ^k\n";
    std::cout << "   Pero ang mga ct ay ~157 digits (Q × r)\n";
    std::cout << "   Ang Q ay 77 digits\n";
    std::cout << "   Ang r ay ~6 digits\n\n";
    
    std::cout << "3. APPROXIMATE Q RECOVERY:\n";
    std::cout << "   ct / r ≈ Q (kung maliit ang r)\n";
    std::cout << "   Kung ang attacker ay mag-divide sa maliit na numbers,\n";
    std::cout << "   baka makita ang Q bilang common factor.\n\n";
    
    // Check kung ang ct ay may common factor sa maliit na range
    std::cout << "4. SMALL FACTOR ATTACK:\n";
    std::cout << "   Check kung ang mga ct ay may common small factors\n";
    std::cout << "   na magre-reveal ng Q\n\n";
    
    // Simple check: ct mod small primes
    std::cout << "   ct0 mod 2 = " << cts[0] % 2 << "\n";
    std::cout << "   ct0 mod 3 = " << cts[0] % 3 << "\n";
    std::cout << "   ct0 mod 5 = " << cts[0] % 5 << "\n";
    std::cout << "   ct0 mod 7 = " << cts[0] % 7 << "\n\n";
    
    std::cout << "5. HONEST ASSESSMENT:\n";
    std::cout << "   Ang Q-embedding ay may SECURITY CONCERN:\n";
    std::cout << "   - Kung ang r ay maliit (6 digits), ang Q ay ~77 digits\n";
    std::cout << "   - Ang ct ay ~83 digits\n";
    std::cout << "   - Attacker ay pwede mag-try ng maliit na divisors\n";
    std::cout << "   - Kung r = 1, ct = Q + message → halos Q!\n";
    std::cout << "   - Kailangan ng MALAKING r para ma-hide ang Q\n\n";
    
    std::cout << "6. FIX: GAMITIN ANG MALAKING R\n";
    std::cout << "   r ay dapat ~same size sa Q (77 digits)\n";
    std::cout << "   ct = m·φ^k + r·Q (r random 77-digit)\n";
    std::cout << "   → Ang ct ay ~154 digits, Q hidden sa noise\n\n";
    
    return 0;
}
