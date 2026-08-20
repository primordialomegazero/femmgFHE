#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  Z[ζ_5] FHE - SINGLE SLOT ENCODING\n";
    std::cout << "  Polynomial representation sa iisang slot\n";
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

    auto decrypt = [&](Ciphertext<DCRTPoly> ct) -> int64_t {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };

    // Sa Z[ζ_5], ang φ = 1.618...
    // Sa finite field Z_65537, kailangan nating hanapin
    // ang element na may period-2 NAND property
    
    // Ang NAND_norm sa Z[ζ_5] ay:
    // NAND_norm(x,y) = (φ - xy + 1) / φ
    // = (φ - xy + 1) × inv_phi
    
    // Sa iisang slot, ito ay simpleng scalar operation:
    // inv_phi = φ - 1
    
    // Hanapin ang φ sa Z_65537 na may property:
    // φ × (φ - 1) = 1 (golden ratio property)
    // φ² = φ + 1
    
    std::cout << "1. HANAPIN ANG φ SA Z_65537:\n";
    std::cout << "   Kailangan: φ² = φ + 1 mod 65537\n";
    std::cout << "-----------------------------------\n";

    int64_t phi = -1;
    for (int64_t i = 1; i < 65537; i++) {
        if ((i * i) % 65537 == (i + 1) % 65537) {
            phi = i;
            break;
        }
    }

    if (phi == -1) {
        std::cout << "   Walang φ sa Z_65537 na may φ² = φ + 1\n";
        std::cout << "   (Dahil √5 ay non-residue sa Z_65537)\n\n";
        
        // Kailangan ng field extension
        std::cout << "2. FIELD EXTENSION Z_65537[√5]:\n";
        std::cout << "   Element: a + b√5 kung saan a,b ∈ Z_65537\n";
        std::cout << "   φ = (1 + √5)/2\n";
        std::cout << "-----------------------------------\n";
        
        // Sa field extension, ang element ay (a, b)
        // φ = (inv2, inv2) kung saan inv2 = 32769
        // (1/2, 1/2) = (32769, 32769)
        
        int64_t inv2 = 32769; // 2 × 32769 = 65538 ≡ 1 mod 65537
        
        std::cout << "   φ = (" << inv2 << ", " << inv2 << ")\n";
        std::cout << "   Ibig sabihin: φ = " << inv2 << " + " << inv2 << "√5\n\n";
        
        // NAND_norm sa field extension:
        // x = (a, b), y = (c, d)
        // xy = (ac + 5bd, ad + bc)
        // φ - xy + 1 = (inv2 - ac - 5bd + 1, inv2 - ad - bc)
        // Result × inv_phi kung saan inv_phi = φ - 1 = (inv2 - 1, inv2)
        
        std::cout << "3. NAND SA FIELD EXTENSION (THEORETICAL):\n";
        std::cout << "   (Kailangan ng 2D encryption)\n";
        std::cout << "-----------------------------------\n";
        
        // Sa 2 slots: slot 0 = a, slot 1 = b
        // Para sa multiplication:
        // (ac + 5bd, ad + bc)
        // Ito ay nangangailangan ng cross-slot multiplication
        // na hindi directly supported sa BFV
        
        std::cout << "   Ang field extension ay nangangailangan ng:\n";
        std::cout << "   - Cross-slot multiplication\n";
        std::cout << "   - Slot rotation (EvalRotate)\n";
        std::cout << "   - Hindi ito kaya ng basic BFV\n\n";
        
    } else {
        std::cout << "   φ = " << phi << "\n";
        std::cout << "   φ² = " << (phi * phi) % 65537 << "\n";
        std::cout << "   φ + 1 = " << (phi + 1) % 65537 << "\n\n";
    }

    std::cout << "========================================\n";
    std::cout << "  KONKLUSYON:\n";
    std::cout << "  - Z_65537 ay walang φ (√5 non-residue)\n";
    std::cout << "  - Field extension Z_65537[√5] ay may φ\n";
    std::cout << "  - Ngunit kailangan ng cross-slot operations\n";
    std::cout << "  - Ito ay hindi directly supported sa BFV\n";
    std::cout << "  - Kailangan ng custom scheme o CKKS na\n";
    std::cout << "    may complex number support\n";
    std::cout << "========================================\n";

    return 0;
}
