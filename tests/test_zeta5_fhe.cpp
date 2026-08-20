#include "openfhe.h"
#include <iostream>
#include <vector>
#include <array>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  Z[ζ_5] FHE INTEGRATION\n";
    std::cout << "  Golden ratio NAND sa encrypted domain\n";
    std::cout << "========================================\n\n";

    // STEP 1: I-embed ang Z[ζ_5] sa plaintext space
    // Ang Z[ζ_5] ay may degree 4
    // I-encode ang bawat coefficient sa separate slots
    
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

    std::cout << "1. ENCODING NG Z[ζ_5] SA BFV:\n";
    std::cout << "   4 slots = 4 coefficients\n";
    std::cout << "-----------------------------------\n";

    // φ = (0, 0, -1, -1) sa Z[ζ_5]
    // Sa mod 65537: -1 = 65536
    std::vector<int64_t> phi_coeff(16384, 0);
    phi_coeff[0] = 0;
    phi_coeff[1] = 0;
    phi_coeff[2] = 65536; // -1
    phi_coeff[3] = 65536; // -1

    std::vector<int64_t> zero_coeff(16384, 0);
    std::vector<int64_t> one_coeff(16384, 0);
    one_coeff[0] = 1;

    auto ct_phi = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(phi_coeff));
    auto ct_zero = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zero_coeff));
    auto ct_one = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(one_coeff));

    auto decrypt = [&](Ciphertext<DCRTPoly> ct) -> std::vector<int64_t> {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue();
    };

    // STEP 2: NAND_norm sa encrypted domain
    // NAND_norm(x,y) = (φ - xy + 1) × inv_phi
    // inv_phi = φ - 1 = (-1, 0, -1, -1)
    
    std::vector<int64_t> inv_phi_coeff(16384, 0);
    inv_phi_coeff[0] = 65536; // -1
    inv_phi_coeff[1] = 0;
    inv_phi_coeff[2] = 65536; // -1
    inv_phi_coeff[3] = 65536; // -1
    
    auto ct_inv_phi = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(inv_phi_coeff));

    auto nand_zeta5 = [&](Ciphertext<DCRTPoly> x, Ciphertext<DCRTPoly> y) {
        // xy
        auto xy = cc->EvalMult(x, y);
        // φ - xy
        auto diff = cc->EvalSub(ct_phi, xy);
        // φ - xy + 1
        auto sum = cc->EvalAdd(diff, ct_one);
        // (φ - xy + 1) × inv_phi
        auto result = cc->EvalMult(sum, ct_inv_phi);
        return result;
    };

    std::cout << "2. NAND_norm TRUTH TABLE (ENCRYPTED):\n";
    std::cout << "-----------------------------------\n";

    auto r00 = nand_zeta5(ct_zero, ct_zero);
    auto r01 = nand_zeta5(ct_zero, ct_phi);
    auto r10 = nand_zeta5(ct_phi, ct_zero);
    auto r11 = nand_zeta5(ct_phi, ct_phi);

    auto d00 = decrypt(r00);
    auto d01 = decrypt(r01);
    auto d10 = decrypt(r10);
    auto d11 = decrypt(r11);

    std::cout << "   NAND(0,0) = (" << d00[0] << "," << d00[1] << "," 
              << d00[2] << "," << d00[3] << ")\n";
    std::cout << "   NAND(0,1) = (" << d01[0] << "," << d01[1] << "," 
              << d01[2] << "," << d01[3] << ")\n";
    std::cout << "   NAND(1,0) = (" << d10[0] << "," << d10[1] << "," 
              << d10[2] << "," << d10[3] << ")\n";
    std::cout << "   NAND(1,1) = (" << d11[0] << "," << d11[1] << "," 
              << d11[2] << "," << d11[3] << ")\n\n";

    // STEP 3: PERIOD-2 VERIFICATION SA ENCRYPTED DOMAIN
    std::cout << "3. PERIOD-2 SA ENCRYPTED DOMAIN:\n";
    std::cout << "-----------------------------------\n";

    auto inner0 = nand_zeta5(ct_zero, ct_zero);
    auto outer0 = nand_zeta5(inner0, inner0);
    auto d_outer0 = decrypt(outer0);
    std::cout << "   NAND²(0) = (" << d_outer0[0] << "," << d_outer0[1] << "," 
              << d_outer0[2] << "," << d_outer0[3] << ")\n";
    std::cout << "   Expected: (0,0,0,0)\n\n";

    auto inner1 = nand_zeta5(ct_phi, ct_phi);
    auto outer1 = nand_zeta5(inner1, inner1);
    auto d_outer1 = decrypt(outer1);
    std::cout << "   NAND²(1) = (" << d_outer1[0] << "," << d_outer1[1] << "," 
              << d_outer1[2] << "," << d_outer1[3] << ")\n";
    std::cout << "   Expected: (0,0,65536,65536) = φ\n\n";

    // STEP 4: DEEP CHAIN
    std::cout << "4. DEEP CHAIN (100 NANDs sa encrypted domain):\n";
    std::cout << "-----------------------------------\n";

    auto current = ct_zero;
    int errors = 0;

    for (int i = 1; i <= 100; i++) {
        current = nand_zeta5(current, current);
        
        auto d = decrypt(current);
        bool is_phi = (d[0] == 0 && d[1] == 0 && d[2] == 65536 && d[3] == 65536);
        bool is_zero = (d[0] == 0 && d[1] == 0 && d[2] == 0 && d[3] == 0);
        
        bool expected_phi = (i % 2 == 1);
        
        if (expected_phi && !is_phi) {
            errors++;
            if (errors <= 3) {
                std::cout << "   Iter " << i << ": FAIL\n";
            }
        }
        if (!expected_phi && !is_zero) {
            errors++;
            if (errors <= 3) {
                std::cout << "   Iter " << i << ": FAIL\n";
            }
        }
        
        if (i % 10 == 0) {
            std::cout << "   " << i << " NANDs: " 
                      << (errors == 0 ? "OK" : "FAIL") << "\n";
        }
    }

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  - Encrypted NAND chain: " 
              << (errors == 0 ? "ALL CORRECT" : "WITH ERRORS") << "\n";
    std::cout << "  - Errors: " << errors << "\n";
    if (errors == 0) {
        std::cout << "  - Z[ζ_5] FHE INTEGRATION SUCCESSFUL\n";
        std::cout << "  - Golden ratio NAND sa encrypted domain\n";
    }
    std::cout << "========================================\n";

    return 0;
}
