#include "openfhe.h"
#include <iostream>
#include <vector>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  GOLDEN RATIO NOISE ANALYSIS\n";
    std::cout << "  φ = (1 + √5) / 2\n";
    std::cout << "  Emergent properties sa noise growth\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetPlaintextModulus(65537);
    parameters.SetMultiplicativeDepth(20);
    parameters.SetRingDim(32768);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    std::vector<int64_t> zeros(16384, 0);
    std::vector<int64_t> ones(16384, 1);

    auto ct0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zeros));
    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(ones));

    auto decrypt = [&](Ciphertext<DCRTPoly> ct) -> int64_t {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };

    auto nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto ab = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, ab);
    };

    // GOLDEN RATIO CONSTANTS
    // φ = 1.618033988749895
    // Sa mod 65537, ang golden ratio ay:
    // φ_mod = (1 + sqrt(5)) / 2 mod 65537
    // sqrt(5) mod 65537 = ?
    
    // Hanapin ang sqrt(5) mod 65537
    int64_t sqrt5 = 0;
    for (int64_t i = 1; i < 65537; i++) {
        if ((i * i) % 65537 == 5) {
            sqrt5 = i;
            break;
        }
    }
    
    std::cout << "sqrt(5) mod 65537 = " << sqrt5 << "\n";
    
    // φ = (1 + sqrt5) / 2 mod 65537
    // Division by 2 = multiply by inverse of 2
    int64_t inv2 = 32769; // 2 * 32769 = 65538 ≡ 1 mod 65537
    int64_t phi = ((1 + sqrt5) * inv2) % 65537;
    
    std::cout << "φ mod 65537 = " << phi << "\n";
    std::cout << "φ² mod 65537 = " << (phi * phi) % 65537 << "\n";
    std::cout << "φ + 1 mod 65537 = " << (phi + 1) % 65537 << "\n";
    std::cout << "Verification: φ² = φ + 1? "
              << ((phi * phi) % 65537 == (phi + 1) % 65537 ? "YES" : "NO") << "\n\n";

    std::cout << "1. GOLDEN RATIO NAND CHAIN:\n";
    std::cout << "   (I-encode ang value sa φ basis)\n";
    std::cout << "-----------------------------------\n";

    // Golden ratio encoding:
    // 0 = 0
    // 1 = φ mod 65537
    // NAND sa golden ratio basis
    
    auto phi_ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({phi}));
    
    auto nand_phi = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto ab = cc->EvalMult(a, b);
        return cc->EvalSub(phi_ct, ab);
    };

    auto current = ct0;
    int errors = 0;
    int max_correct = 0;

    for (int i = 1; i <= 40; i++) {
        current = nand_phi(current, current);
        
        int result = decrypt(current);
        // Sa golden ratio basis, ang expected ay:
        // odd iterations: φ, even iterations: 0
        int expected = (i % 2 == 0) ? 0 : phi;
        
        if (result == expected) {
            max_correct = i;
        } else {
            errors++;
            if (errors <= 10) {
                std::cout << "   Iter " << i << ": " << result
                          << " (expected " << expected << ") FAIL\n";
            }
        }
    }

    std::cout << "   Max correct: " << max_correct << " NANDs\n";
    std::cout << "   Errors: " << errors << "\n";

    std::cout << "\n2. FIBONACCI NOISE COMPENSATION:\n";
    std::cout << "   (Gamitin ang Fibonacci sequence\n";
    std::cout << "    para i-compensate ang noise)\n";
    std::cout << "-----------------------------------\n";

    // Fibonacci numbers mod 65537
    std::vector<int64_t> fib(50);
    fib[0] = 0;
    fib[1] = 1;
    for (int i = 2; i < 50; i++) {
        fib[i] = (fib[i-1] + fib[i-2]) % 65537;
    }

    current = ct0;
    errors = 0;
    max_correct = 0;

    for (int i = 1; i <= 30; i++) {
        current = nand(current, current);
        
        // Noise compensation gamit ang Fibonacci
        // Kung ang noise ay lumalaki sa Fibonacci pattern,
        // pwede nating ibawas ito
        if (i >= 3 && fib[i] != 0) {
            auto fib_ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({fib[i]}));
            current = cc->EvalSub(current, fib_ct);
        }
        
        int expected = (i % 2 == 0) ? 0 : 1;
        int result = decrypt(current);
        if (result == expected) {
            max_correct = i;
        } else {
            errors++;
            if (errors <= 10) {
                std::cout << "   Iter " << i << ": " << result
                          << " (expected " << expected << ") FAIL\n";
            }
        }
    }

    std::cout << "   Max correct: " << max_correct << " NANDs\n";
    std::cout << "   Errors: " << errors << "\n";

    std::cout << "\n3. GOLDEN RATIO RESONANCE:\n";
    std::cout << "   (φⁿ mod 65537 pattern)\n";
    std::cout << "-----------------------------------\n";

    std::cout << "   φ¹ = " << phi << "\n";
    std::cout << "   φ² = " << (phi * phi) % 65537 << "\n";
    std::cout << "   φ³ = " << (phi * phi * phi) % 65537 << "\n";
    std::cout << "   φ⁴ = " << (phi * phi * phi * phi) % 65537 << "\n";
    std::cout << "   φ⁵ = " << (phi * phi * phi * phi * phi) % 65537 << "\n";
    std::cout << "   φ⁶ = " << (phi * phi * phi * phi * phi * phi) % 65537 << "\n";
    std::cout << "   φ⁷ = " << (phi * phi * phi * phi * phi * phi * phi) % 65537 << "\n";
    std::cout << "   φ⁸ = " << (phi * phi * phi * phi * phi * phi * phi * phi) % 65537 << "\n";

    // Hanapin ang period ng φ mod 65537
    int64_t current_phi = phi;
    int period = 0;
    for (int i = 1; i <= 65537; i++) {
        current_phi = (current_phi * phi) % 65537;
        if (current_phi == phi) {
            period = i;
            break;
        }
    }
    std::cout << "\n   Period ng φ mod 65537: " << period << "\n";

    std::cout << "\n========================================\n";
    std::cout << "  KEY INSIGHT:\n";
    std::cout << "  - φ mod 65537 = " << phi << "\n";
    std::cout << "  - Period: " << period << "\n";
    std::cout << "  - Kung ang noise ay may golden ratio\n";
    std::cout << "    structure, baka may resonance\n";
    std::cout << "========================================\n";

    return 0;
}
