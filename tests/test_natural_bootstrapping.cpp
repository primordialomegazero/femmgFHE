// NATURAL HOMOMORPHIC BOOTSTRAPPING SEARCH
// Hanapin ang φ-native na paraan para ma-refresh ang noise

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "NATURAL HOMOMORPHIC BOOTSTRAPPING SEARCH\n";
    std::cout << "========================================\n\n";

    NTL::ZZ Q = NTL::to_ZZ("32317006071311007300714876688669951960444102669715484032130345427524655138867890893197201411522913463688717960921898019494119559150490921095088152386448283120630877367300996091750197750389652106796057638384067568276792218642619756161838094338476170470581645852036305042887575891541065808607552399123930385521914333389668342420684974786564569494856176035326322058077805659331026192708460314150258592864177116725943603718461857357598351152301645904403697613233287231227125684710820209725157101726931323469678542580656697935045997268352998638215525166389437335543602135433229604645318478604952148193555853611059596217801");

    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;

    long k = 2048;
    NTL::ZZ phi_k = NTL::to_ZZ(1);
    NTL::ZZ psi_k = NTL::to_ZZ(1);
    for (long i = 0; i < k; i++) { phi_k = (phi_k * phi) % Q; psi_k = (psi_k * psi) % Q; }
    NTL::ZZ L_k = (phi_k + psi_k) % Q;
    NTL::ZZ inv_L_k = NTL::InvMod(L_k, Q);

    std::cout << "1. ANG BOOTSTRAPPING SA FHE:\n";
    std::cout << "   - Kunin ang encrypted ciphertext na may noise\n";
    std::cout << "   - I-evaluate ang DECRYPTION CIRCUIT homomorphically\n";
    std::cout << "   - Result: refreshed ciphertext na may maliit na noise\n\n";

    std::cout << "2. ANG DECRYPTION CIRCUIT SA φ-STRUCTURE:\n";
    std::cout << "   v = ct mod Q\n";
    std::cout << "   scaled = v · inv_L_k\n";
    std::cout << "   Kung scaled ≈ 0: message 0\n";
    std::cout << "   Kung scaled ≈ 1: message 1\n\n";

    std::cout << "3. ANG KEY CHALLENGE:\n";
    std::cout << "   Ang decryption circuit ay kailangan i-evaluate\n";
    std::cout << "   nang homomorphically — sa ENCRYPTED DOMAIN.\n";
    std::cout << "   → Ito ay BOOTSTRAPPING mismo!\n\n";

    std::cout << "4. ANG φ-NATIVE NA APPROACH:\n";
    std::cout << "   Ang NAND ay may PERIOD-2 property:\n";
    std::cout << "   NAND(NAND(x,x), NAND(x,x)) = x\n";
    std::cout << "   → Kung mag-NAND tayo ng 2 beses,\n";
    std::cout << "     babalik tayo sa original!\n\n";

    std::cout << "5. ANG EMERGENT PROPERTY:\n";
    std::cout << "   L(2k) = L(k)² - 2\n";
    std::cout << "   → Kung i-compute natin ang L(2k),\n";
    std::cout << "     may natural na noise reduction?\n\n";

    // Subukan: NAND ng 2 beses = identity?
    NTL::ZZ x = L_k;  // message 1
    auto nand = [&](NTL::ZZ a) {
        NTL::ZZ prod = (a * a) % Q;
        NTL::ZZ scaled = (prod * inv_L_k) % Q;
        NTL::ZZ result = (L_k - scaled) % Q;
        if (result < 0) result += Q;
        return result;
    };

    NTL::ZZ n1 = nand(x);
    NTL::ZZ n2 = nand(n1);

    std::cout << "6. NAND(NAND(x,x), NAND(x,x)) TEST:\n";
    std::cout << "   x = " << x << "\n";
    std::cout << "   NAND(x,x) = " << n1 << "\n";
    std::cout << "   NAND(NAND(x,x), NAND(x,x)) = " << n2 << "\n";
    std::cout << "   Match x: " << (n2 == x ? "YES ✓" : "NO ✗") << "\n\n";

    std::cout << "7. KEY INSIGHT:\n";
    std::cout << "   Ang period-2 property ay nagbibigay ng\n";
    std::cout << "   natural na noise cancellation pagkatapos ng 2 NANDs.\n";
    std::cout << "   Kung may noise, ang 2-NAND cycle ay nagre-reset!\n\n";

    // Test with noise
    std::mt19937_64 rng(42);
    auto encrypt_noisy = [&](bool bit, NTL::ZZ e) {
        NTL::ZZ r = NTL::RandomBnd(Q);
        NTL::ZZ m_val = bit ? L_k : NTL::to_ZZ(0);
        return m_val + e * psi_k + r * Q;
    };

    auto decrypt = [&](NTL::ZZ ct) {
        NTL::ZZ v = ct % Q;
        if (v < 0) v += Q;
        NTL::ZZ d_L = (v > L_k) ? v - L_k : L_k - v;
        if (d_L > Q/2) d_L = Q - d_L;
        NTL::ZZ d_0 = (v < Q/2) ? v : Q - v;
        return d_L < d_0;
    };

    std::cout << "8. 2-NAND CYCLE WITH NOISE:\n";
    for (long e : {1L, 5L, 10L}) {
        auto x_noisy = encrypt_noisy(true, NTL::to_ZZ(e));
        auto n1_noisy = nand(x_noisy);
        auto n2_noisy = nand(n1_noisy);
        bool recovered = decrypt(n2_noisy);
        std::cout << "   e=" << e << ": NAND(NAND(x)) = " << recovered 
                  << " (exp 1)\n";
    }

    return 0;
}
