// OPENFHE AUTO PARAMETERS — Automatic Parameter Selection
#include <NTL/ZZ.h>
#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  OPENFHE AUTO PARAMETERS\n";
    std::cout << "  Automatic Parameter Selection\n";
    std::cout << "========================================\n\n";

    // Automatic parameter generation
    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetPlaintextModulus(536903681);  // Mas malaking prime
    parameters.SetMultiplicativeDepth(30);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto ring_dim = cc->GetRingDimension();
    std::cout << "BFV Context (auto-generated):\n";
    std::cout << "  Plaintext modulus: " << cc->GetEncodingParams()->GetPlaintextModulus() << "\n";
    std::cout << "  Ring dimension: " << ring_dim << "\n";
    std::cout << "  Multiplicative depth: 30\n\n";

    // φ-Structure — gamitin ang plaintext modulus mula sa context
    NTL::ZZ p_mod = NTL::to_ZZ(cc->GetEncodingParams()->GetPlaintextModulus());
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

    std::cout << "φ-Structure:\n";
    std::cout << "  φ^k = " << phi_k_plain << "\n";
    std::cout << "  ψ^k = " << psi_k_plain << "\n";
    std::cout << "  inv_φ^k = " << inv_phi_k_plain << "\n";
    std::cout << "  offset = " << offset_plain << "\n\n";

    // Precompute encrypted constants
    int vec_size = ring_dim;
    std::vector<int64_t> vec_phi_k(vec_size, 0);
    vec_phi_k[0] = NTL::to_long(phi_k_plain % NTL::to_ZZ(536903681));

    std::vector<int64_t> vec_inv_phi_k(vec_size, 0);
    vec_inv_phi_k[0] = NTL::to_long(inv_phi_k_plain % NTL::to_ZZ(536903681));

    std::vector<int64_t> vec_zero(vec_size, 0);
    std::vector<int64_t> vec_one(vec_size, 0);
    vec_one[0] = NTL::to_long(phi_k_plain % NTL::to_ZZ(536903681));

    auto ct_phi_k = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vec_phi_k));
    auto ct_inv_phi_k = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vec_inv_phi_k));
    auto ct_zero = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vec_zero));
    auto ct_one = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vec_one));

    // Gate definitions
    auto homomorphic_nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto prod = cc->EvalMult(a, b);
        auto scaled = cc->EvalMult(prod, ct_inv_phi_k);
        return cc->EvalSub(ct_phi_k, scaled);
    };

    auto homomorphic_not = [&](Ciphertext<DCRTPoly> a) {
        return homomorphic_nand(a, a);
    };

    auto homomorphic_and = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto nand = homomorphic_nand(a, b);
        return homomorphic_nand(nand, nand);
    };

    auto homomorphic_or = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto not_a = homomorphic_not(a);
        auto not_b = homomorphic_not(b);
        return homomorphic_nand(not_a, not_b);
    };

    auto homomorphic_xor = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto nand_ab = homomorphic_nand(a, b);
        auto nand_a_nab = homomorphic_nand(a, nand_ab);
        auto nand_b_nab = homomorphic_nand(b, nand_ab);
        return homomorphic_nand(nand_a_nab, nand_b_nab);
    };

    auto decrypt_to_bool = [&](Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        auto val = pt->GetPackedValue()[0];
        long long target = NTL::to_long(phi_k_plain % NTL::to_ZZ(536903681));
        return (val == target) ? 1 : 0;
    };

    std::cout << "ALL GATES READY\n\n";

    // ============================================
    // TEST 1: DEEPEST CHAIN (30 gates)
    // ============================================
    std::cout << "TEST 1: DEEPEST CHAIN (30 gates)\n";
    std::cout << "==================================\n\n";

    auto current = ct_one;
    int errors = 0;

    for (int gate = 0; gate < 30; gate++) {
        current = homomorphic_nand(current, current);

        int expected = (gate % 2 == 0) ? 0 : 1;
        int got = decrypt_to_bool(current);

        if (expected != got) errors++;

        if (gate < 10 || gate >= 25) {
            std::cout << "  Gate " << gate << ": expected=" << expected
                      << " got=" << got
                      << (expected == got ? " YES" : " NO") << "\n";
        }
    }

    std::cout << "\n  Deep chain: " << errors << "/30 errors ("
              << (100.0 * (30 - errors) / 30) << "%)\n\n";

    // ============================================
    // TEST 2: FULL ADDER
    // ============================================
    std::cout << "TEST 2: FULL ADDER\n";
    std::cout << "====================\n\n";

    std::vector<Ciphertext<DCRTPoly>> inputs = {ct_zero, ct_one};
    int adder_errors = 0;
    int adder_total = 0;

    for (auto a : inputs) {
        for (auto b : inputs) {
            for (auto cin : inputs) {
                auto a_xor_b = homomorphic_xor(a, b);
                auto sum = homomorphic_xor(a_xor_b, cin);

                auto a_and_b = homomorphic_and(a, b);
                auto cin_and_xor = homomorphic_and(cin, a_xor_b);
                auto cout = homomorphic_or(a_and_b, cin_and_xor);

                int sum_got = decrypt_to_bool(sum);
                int cout_got = decrypt_to_bool(cout);
                int a_val = decrypt_to_bool(a);
                int b_val = decrypt_to_bool(b);
                int cin_val = decrypt_to_bool(cin);
                int sum_expected = a_val ^ b_val ^ cin_val;
                int cout_expected = (a_val & b_val) | (cin_val & (a_val ^ b_val));

                if (sum_got != sum_expected || cout_got != cout_expected) adder_errors++;
                adder_total++;
            }
        }
    }

    std::cout << "  Full Adder: " << adder_errors << "/" << adder_total << " errors ("
              << (100.0 * (adder_total - adder_errors) / adder_total) << "%)\n";

    return 0;
}
