// OPENFHE PERIOD-2 — Real FHE Library Integration
#include <NTL/ZZ.h>
#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  OPENFHE PERIOD-2 INTEGRATION\n";
    std::cout << "  Real FHE Library + φ-Structure\n";
    std::cout << "========================================\n\n";

    // ============================================
    // STEP 1: Setup BFV Context
    // ============================================
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

    std::cout << "BFV Context ready:\n";
    std::cout << "  Plaintext modulus: 65537\n";
    std::cout << "  Ring dimension: 32768\n";
    std::cout << "  Multiplicative depth: 20\n\n";

    // ============================================
    // STEP 2: φ-STRUCTURE SA LOOB NG BFV
    // ============================================
    NTL::ZZ p_mod = NTL::to_ZZ(65537);
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), p_mod);
    NTL::ZZ inv2_plain = NTL::InvMod(NTL::to_ZZ(2), p_mod);
    NTL::ZZ phi_plain = ((NTL::to_ZZ(1) + sqrt5) * inv2_plain) % p_mod;
    NTL::ZZ psi_plain = (NTL::to_ZZ(1) - phi_plain + p_mod) % p_mod;

    NTL::ZZ phi_k_plain = NTL::to_ZZ(1);
    NTL::ZZ psi_k_plain = NTL::to_ZZ(1);
    for (int i = 0; i < 42; i++) {
        phi_k_plain = (phi_k_plain * phi_plain) % p_mod;
        psi_k_plain = (psi_k_plain * psi_plain) % p_mod;
    }
    NTL::ZZ inv_phi_k_plain = NTL::InvMod(phi_k_plain, p_mod);
    NTL::ZZ inv_psi_k_plain = NTL::InvMod(psi_k_plain, p_mod);
    NTL::ZZ offset_plain = (phi_k_plain * inv_psi_k_plain) % p_mod;

    std::cout << "φ-Structure sa Z_65537:\n";
    std::cout << "  φ = " << phi_plain << "\n";
    std::cout << "  ψ = " << psi_plain << "\n";
    std::cout << "  φ^k = " << phi_k_plain << "\n";
    std::cout << "  ψ^k = " << psi_k_plain << "\n";
    std::cout << "  offset = " << offset_plain << "\n\n";

    // ============================================
    // STEP 3: BFV ENCRYPTION
    // ============================================
    std::vector<int64_t> vec_phi_k(32768, 0);
    vec_phi_k[0] = NTL::to_long(phi_k_plain);

    std::vector<int64_t> vec_inv_phi_k(32768, 0);
    vec_inv_phi_k[0] = NTL::to_long(inv_phi_k_plain);

    auto ct_phi_k = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vec_phi_k));
    auto ct_inv_phi_k = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vec_inv_phi_k));

    std::cout << "BFV ciphertexts ready\n\n";

    // ============================================
    // STEP 4: HOMOMORPHIC NAND
    // ============================================
    auto homomorphic_nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto prod = cc->EvalMult(a, b);
        auto scaled = cc->EvalMult(prod, ct_inv_phi_k);
        auto result = cc->EvalSub(ct_phi_k, scaled);
        return result;
    };

    std::cout << "Homomorphic NAND ready\n\n";

    // ============================================
    // STEP 5: TRUTH TABLE TEST
    // ============================================
    std::cout << "BFV HOMOMORPHIC NAND TEST\n";
    std::cout << "===========================\n\n";

    std::vector<int64_t> vec_zero(32768, 0);
    std::vector<int64_t> vec_one(32768, 0);
    vec_one[0] = NTL::to_long(phi_k_plain);

    auto ct_zero = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vec_zero));
    auto ct_one = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vec_one));

    // Test: NAND(0,0) = φ^k (1)
    std::cout << "  NAND(0,0):\n";
    auto nand_00 = homomorphic_nand(ct_zero, ct_zero);
    Plaintext pt_00;
    cc->Decrypt(keys.secretKey, nand_00, &pt_00);
    auto val_00 = pt_00->GetPackedValue()[0];
    std::cout << "    Result: " << val_00 << " (expected " << phi_k_plain << ")\n";
    std::cout << "    Correct: " << (val_00 == NTL::to_long(phi_k_plain) ? "YES" : "NO") << "\n\n";

    // Test: NAND(1,1) = 0
    std::cout << "  NAND(1,1):\n";
    auto nand_11 = homomorphic_nand(ct_one, ct_one);
    Plaintext pt_11;
    cc->Decrypt(keys.secretKey, nand_11, &pt_11);
    auto val_11 = pt_11->GetPackedValue()[0];
    std::cout << "    Result: " << val_11 << " (expected 0)\n";
    std::cout << "    Correct: " << (val_11 == 0 ? "YES" : "NO") << "\n\n";

    // Test: NAND(0,1) = φ^k (1)
    std::cout << "  NAND(0,1):\n";
    auto nand_01 = homomorphic_nand(ct_zero, ct_one);
    Plaintext pt_01;
    cc->Decrypt(keys.secretKey, nand_01, &pt_01);
    auto val_01 = pt_01->GetPackedValue()[0];
    std::cout << "    Result: " << val_01 << " (expected " << phi_k_plain << ")\n";
    std::cout << "    Correct: " << (val_01 == NTL::to_long(phi_k_plain) ? "YES" : "NO") << "\n\n";

    // Test: NAND(1,0) = φ^k (1)
    std::cout << "  NAND(1,0):\n";
    auto nand_10 = homomorphic_nand(ct_one, ct_zero);
    Plaintext pt_10;
    cc->Decrypt(keys.secretKey, nand_10, &pt_10);
    auto val_10 = pt_10->GetPackedValue()[0];
    std::cout << "    Result: " << val_10 << " (expected " << phi_k_plain << ")\n";
    std::cout << "    Correct: " << (val_10 == NTL::to_long(phi_k_plain) ? "YES" : "NO") << "\n\n";

    // ============================================
    // STEP 6: DEEP CHAIN (10 gates)
    // ============================================
    std::cout << "BFV DEEP CHAIN (10 gates):\n";
    std::cout << "===========================\n\n";

    auto current = ct_one;
    std::cout << "  Gate | Expected | Got | OK?\n";
    std::cout << "  -----|----------|-----|-----\n";

    int errors = 0;
    for (int gate = 0; gate < 10; gate++) {
        current = homomorphic_nand(current, current);

        Plaintext pt;
        cc->Decrypt(keys.secretKey, current, &pt);
        auto val = pt->GetPackedValue()[0];

        int expected = (gate % 2 == 0) ? 0 : 1;
        int got = (val == NTL::to_long(phi_k_plain)) ? 1 : 0;

        if (expected != got) errors++;

        std::cout << "  " << gate << "    | " << expected
                  << "        | " << got
                  << "   | " << (expected == got ? "YES" : "NO") << "\n";
    }

    std::cout << "\n  Errors: " << errors << "/10\n";
    std::cout << "  Accuracy: " << (100.0 * (10 - errors) / 10) << "%\n";

    return 0;
}
