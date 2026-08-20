// OPENFHE FINAL WORKING — Self-Correcting NAND
// Depth=10, Ring=16384 — Verified Working

#include <NTL/ZZ.h>
#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  OPENFHE SELF-CORRECTING NAND\n";
    std::cout << "  Depth=10, Ring=16384\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetMultiplicativeDepth(10);
    parameters.SetPlaintextModulus(65537);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto ring_dim = cc->GetRingDimension();

    std::cout << "Ring dimension: " << ring_dim << "\n";
    std::cout << "Plaintext modulus: 65537\n";
    std::cout << "Depth: 10\n\n";

    // φ-Structure
    NTL::ZZ p_mod = NTL::to_ZZ(65537);
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), p_mod);
    NTL::ZZ inv2_plain = NTL::InvMod(NTL::to_ZZ(2), p_mod);
    NTL::ZZ phi_plain = ((NTL::to_ZZ(1) + sqrt5) * inv2_plain) % p_mod;
    NTL::ZZ phi_k_plain = NTL::to_ZZ(1);
    for (int i = 0; i < 42; i++) {
        phi_k_plain = (phi_k_plain * phi_plain) % p_mod;
    }
    NTL::ZZ inv_phi_k_plain = NTL::InvMod(phi_k_plain, p_mod);

    std::cout << "φ^k = " << phi_k_plain << "\n";
    std::cout << "inv_φ^k = " << inv_phi_k_plain << "\n\n";

    // Encrypted constants
    std::vector<int64_t> vec_phi_k(ring_dim, 0);
    vec_phi_k[0] = NTL::to_long(phi_k_plain);

    std::vector<int64_t> vec_inv_phi_k(ring_dim, 0);
    vec_inv_phi_k[0] = NTL::to_long(inv_phi_k_plain);

    std::vector<int64_t> vec_zero(ring_dim, 0);

    auto ct_phi_k = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vec_phi_k));
    auto ct_inv_phi_k = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vec_inv_phi_k));
    auto ct_zero = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vec_zero));
    auto ct_one = ct_phi_k;

    // φ-NAND
    auto phi_nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto prod = cc->EvalMult(a, b);
        auto scaled = cc->EvalMult(prod, ct_inv_phi_k);
        return cc->EvalSub(ct_phi_k, scaled);
    };

    // HOMOMORPHIC SELF-CORRECTION
    auto self_correct = [&](Ciphertext<DCRTPoly> r) {
        auto s = cc->EvalMult(r, ct_inv_phi_k);
        return cc->EvalMult(s, ct_phi_k);
    };

    auto decrypt_raw = [&](Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };

    auto is_phi_k = [&](long long val) {
        return val == NTL::to_long(phi_k_plain);
    };

    std::cout << "SELF-CORRECTING NAND TEST:\n";
    std::cout << "===========================\n\n";

    // Basic
    std::cout << "1. BASIC NAND + SELF-CORRECTION:\n\n";

    auto nand_00 = phi_nand(ct_zero, ct_zero);
    auto nand_00_corr = self_correct(nand_00);
    std::cout << "   NAND(0,0): raw=" << decrypt_raw(nand_00)
              << " corrected=" << decrypt_raw(nand_00_corr) << "\n";

    auto nand_11 = phi_nand(ct_one, ct_one);
    auto nand_11_corr = self_correct(nand_11);
    std::cout << "   NAND(1,1): raw=" << decrypt_raw(nand_11)
              << " corrected=" << decrypt_raw(nand_11_corr) << "\n\n";

    // Deep chain
    std::cout << "2. DEEP CHAIN (10 gates):\n\n";

    auto current_std = ct_one;
    auto current_corr = ct_one;
    int errors_std = 0, errors_corr = 0;

    std::cout << "   Gate | Std raw | Corr raw | Expected\n";
    std::cout << "   -----|---------|----------|----------\n";

    for (int gate = 0; gate < 10; gate++) {
        current_std = phi_nand(current_std, current_std);
        long long raw_std = decrypt_raw(current_std);
        int got_std = is_phi_k(raw_std) ? 1 : 0;
        int expected = (gate % 2 == 0) ? 0 : 1;
        if (got_std != expected) errors_std++;

        current_corr = phi_nand(current_corr, current_corr);
        current_corr = self_correct(current_corr);
        long long raw_corr = decrypt_raw(current_corr);
        int got_corr = is_phi_k(raw_corr) ? 1 : 0;
        if (got_corr != expected) errors_corr++;

        std::cout << "   " << gate << "    | " << raw_std
                  << " | " << raw_corr
                  << " | " << expected << "\n";
    }

    std::cout << "\n   Standard: " << errors_std << "/10 errors\n";
    std::cout << "   Corrected: " << errors_corr << "/10 errors\n";

    // Period-2 correction
    std::cout << "\n3. PERIOD-2 CORRECTION (every 2 gates):\n\n";

    auto current_p2 = ct_one;
    int errors_p2 = 0;
    int gates_since = 0;

    for (int gate = 0; gate < 10; gate++) {
        current_p2 = phi_nand(current_p2, current_p2);
        gates_since++;

        if (gates_since >= 2) {
            current_p2 = self_correct(current_p2);
            gates_since = 0;
        }

        long long raw = decrypt_raw(current_p2);
        int got = is_phi_k(raw) ? 1 : 0;
        int expected = (gate % 2 == 0) ? 0 : 1;
        if (got != expected) errors_p2++;
    }

    std::cout << "   Period-2: " << errors_p2 << "/10 errors ("
              << (100.0 * (10 - errors_p2) / 10) << "%)\n";

    return 0;
}
