#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  Z[ζ_5] FHE - GOLDEN RATIO NAND\n";
    std::cout << "  Plaintext modulus na may φ\n";
    std::cout << "========================================\n\n";

    // p=40961 kung saan:
    // √5 = 19424, φ = 30193
    // φ² = φ + 1 = 30194
    // p - 1 = 40960 = 5 × 8192
    // Kailangan: 2N divides 40960
    // N = 8192, RingDim = 16384

    uint32_t plaintext_modulus = 40961;
    uint32_t ring_dim = 16384;
    uint32_t depth = 10;

    std::cout << "1. PARAMETERS:\n";
    std::cout << "   Plaintext modulus p = " << plaintext_modulus << "\n";
    std::cout << "   Ring dimension = " << ring_dim << "\n";
    std::cout << "   Multiplicative depth = " << depth << "\n";
    std::cout << "   √5 = 19424, φ = 30193\n\n";

    try {
        CCParams<CryptoContextBFVRNS> parameters;
        parameters.SetPlaintextModulus(plaintext_modulus);
        parameters.SetMultiplicativeDepth(depth);
        parameters.SetRingDim(ring_dim);

        CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);

        auto keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);

        std::vector<int64_t> zeros(ring_dim / 2, 0);
        std::vector<int64_t> ones(ring_dim / 2, 0);
        ones[0] = 1;
        
        std::vector<int64_t> phi_vec(ring_dim / 2, 0);
        phi_vec[0] = 30193; // φ = 30193 sa Z_40961

        auto ct_zero = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zeros));
        auto ct_one = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(ones));
        auto ct_phi = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(phi_vec));

        auto decrypt = [&](Ciphertext<DCRTPoly> ct) -> int64_t {
            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct, &pt);
            return pt->GetPackedValue()[0];
        };

        std::cout << "2. GOLDEN RATIO VERIFICATION:\n";
        std::cout << "   φ = " << decrypt(ct_phi) << "\n";
        std::cout << "   φ² = " << (30193 * 30193) % 40961 << "\n";
        std::cout << "   φ + 1 = " << (30193 + 1) % 40961 << "\n";
        std::cout << "   Match: " << ((30193 * 30193) % 40961 == (30193 + 1) % 40961 ? "YES" : "NO") << "\n\n";

        int64_t inv_phi = 30192; // φ - 1
        std::vector<int64_t> inv_phi_vec(ring_dim / 2, 0);
        inv_phi_vec[0] = inv_phi;
        auto ct_inv_phi = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(inv_phi_vec));

        auto nand_phi = [&](Ciphertext<DCRTPoly> x, Ciphertext<DCRTPoly> y) {
            auto xy = cc->EvalMult(x, y);
            auto diff = cc->EvalSub(ct_phi, xy);
            auto sum = cc->EvalAdd(diff, ct_one);
            return cc->EvalMult(sum, ct_inv_phi);
        };

        std::cout << "3. NAND TRUTH TABLE (ENCRYPTED):\n";
        std::cout << "-----------------------------------\n";
        
        auto r00 = nand_phi(ct_zero, ct_zero);
        auto r01 = nand_phi(ct_zero, ct_phi);
        auto r10 = nand_phi(ct_phi, ct_zero);
        auto r11 = nand_phi(ct_phi, ct_phi);

        std::cout << "   NAND(0,0) = " << decrypt(r00) << " (expected " << 30193 << ")\n";
        std::cout << "   NAND(0,1) = " << decrypt(r01) << " (expected " << 30193 << ")\n";
        std::cout << "   NAND(1,0) = " << decrypt(r10) << " (expected " << 30193 << ")\n";
        std::cout << "   NAND(1,1) = " << decrypt(r11) << " (expected " << 0 << ")\n\n";

        std::cout << "4. PERIOD-2 VERIFICATION:\n";
        std::cout << "-----------------------------------\n";
        
        auto inner0 = nand_phi(ct_zero, ct_zero);
        auto outer0 = nand_phi(inner0, inner0);
        std::cout << "   NAND²(0) = " << decrypt(outer0) << " (expected 0)\n";
        
        auto inner1 = nand_phi(ct_phi, ct_phi);
        auto outer1 = nand_phi(inner1, inner1);
        std::cout << "   NAND²(1) = " << decrypt(outer1) << " (expected " << 30193 << ")\n\n";

        std::cout << "5. DEEP CHAIN:\n";
        std::cout << "-----------------------------------\n";
        
        auto current = ct_zero;
        int errors = 0;
        int max_correct = 0;
        
        for (int i = 1; i <= 30; i++) {
            current = nand_phi(current, current);
            int result = decrypt(current);
            int expected = (i % 2 == 0) ? 0 : 30193;
            
            if (result == expected) {
                max_correct = i;
            } else {
                errors++;
                if (errors <= 5) {
                    std::cout << "   Iter " << i << ": " << result
                              << " (expected " << expected << ") FAIL\n";
                }
                break;
            }
            
            if (i % 5 == 0) {
                std::cout << "   " << i << " NANDs: OK\n";
            }
        }
        
        std::cout << "\n   Max correct: " << max_correct << " NANDs\n";
        std::cout << "   Errors: " << errors << "\n";

        std::cout << "\n========================================\n";
        std::cout << "  RESULT:\n";
        std::cout << "  - Golden ratio NAND sa FHE: ";
        if (max_correct > 23) {
            std::cout << "LUMAMPAS SA 23 LIMIT\n";
        } else if (max_correct == 23) {
            std::cout << "PAREHO SA REGULAR BFV\n";
        } else {
            std::cout << "MAS MAIKLI SA REGULAR BFV\n";
        }
        std::cout << "  - Max NANDs: " << max_correct << "\n";
        std::cout << "========================================\n";

    } catch (const std::exception& e) {
        std::cout << "ERROR: " << e.what() << "\n";
    }

    return 0;
}
