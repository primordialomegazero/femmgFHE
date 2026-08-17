// SECURITY ANALYSIS ng RANDOM Q-EMBEDDING
#include <NTL/ZZ.h>
#include <iostream>
#include <vector>
#include <random>

int main() {
    std::cout << "Q-EMBEDDING SECURITY ANALYSIS\n";
    std::cout << "=============================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    
    std::mt19937_64 rng(42);
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    
    // Generate 100 ciphertexts
    std::vector<NTL::ZZ> cts;
    for (int i = 0; i < 100; i++) {
        NTL::ZZ r = NTL::to_ZZ(rng() % 1000000 + 1);
        NTL::ZZ m = (i % 2 == 0) ? NTL::to_ZZ(0) : phi;
        cts.push_back(m + r * Q);
    }
    
    std::cout << "1. Ciphertext Distribution:\n";
    std::cout << "   ct0 = " << cts[0] << "\n";
    std::cout << "   ct1 = " << cts[1] << "\n";
    std::cout << "   ct0 mod Q = " << cts[0] % Q << "\n";
    std::cout << "   ct1 mod Q = " << cts[1] % Q << "\n\n";
    
    std::cout << "2. ATTACKER'S VIEW:\n";
    std::cout << "   Nakikita: ct0, ct1, ct2, ...\n";
    std::cout << "   Hindi alam: Q, φ\n";
    std::cout << "   Ang ct mod Q ay nagbibigay ng m·φ\n";
    std::cout << "   Pero hindi alam ang Q!\n";
    std::cout << "   Kung alam ang Q: ct mod Q = m·φ → message revealed\n";
    std::cout << "   Kung hindi alam ang Q: ct ay random-looking\n\n";
    
    std::cout << "3. SECURITY CONDITION:\n";
    std::cout << "   - Q ay SECRET (hindi public)\n";
    std::cout << "   - φ ay SECRET (hindi public)\n";
    std::cout << "   - Attacker ay may ciphertexts pero hindi Q at φ\n";
    std::cout << "   - Ang Q-embedding ay parang ONE-TIME PAD\n";
    std::cout << "   - Security: information-theoretic kung Q ay random\n\n";
    
    std::cout << "4. KEY INSIGHT:\n";
    std::cout << "   Hindi ito asymmetric (public-key) FHE.\n";
    std::cout << "   Ito ay SYMMETRIC FHE (parehong key para sa encrypt/decrypt).\n";
    std::cout << "   Kung ang φ at Q ay secret, ang scheme ay parang OTP.\n";
    std::cout << "   Kung public ang φ at Q, walang security.\n\n";
    
    std::cout << "5. NEXT STEP:\n";
    std::cout << "   Gawing ASYMMETRIC: publish Q at φ, pero may trapdoor.\n";
    std::cout << "   Trapdoor: DLP — given φ^k, hanapin k.\n";
    std::cout << "   → Ito ay kailangan para maging public-key FHE.\n";
    
    return 0;
}
