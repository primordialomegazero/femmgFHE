// OPENFHE φ-NAND — φ-Structure sa BFV
#include <NTL/ZZ.h>
#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  OPENFHE φ-NAND\n";
    std::cout << "  φ-Structure sa BFV\n";
    std::cout << "========================================\n\n";

    // Setup na gumana na
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
    auto plaintext_mod = cc->GetEncodingParams()->GetPlaintextModulus();

    std::cout << "BFV Context:\n";
    std::cout << "  Plaintext modulus: " << plaintext_mod << "\n";
    std::cout << "  Ring dimension: " << ring_dim << "\n";
    std::cout << "  Multiplicative depth: 10\n\n";

    // φ-Structure sa Z_65537
    NTL::ZZ p_mod = NTL::to_ZZ(65537);
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), p_mod);
    NTL::ZZ inv2_plain = NTL::InvMod(NTL::to_ZZ(2), p_mod);
    NTL::ZZ phi_plain = ((NTL::to_ZZ(1) + sqrt5) * inv2_plain) % p_mod;
    NTL::ZZ psi_plain = (NTL::to_ZZ(1) - phi_plain + p_mod) % p_mod;

    NTL::ZZ phi_k_plain = NTL::to_ZZ(1);
    for (int i = 0; i < 42; i++) {
        phi_k_plain = (phi_k_plain * phi_plain) % p_mod;
    }
    NTL::ZZ inv_phi_k_plain = NTL::InvMod(phi_k_plain, p_mod);

    std::cout << "φ-Structure:\n";
    std::cout << "  φ^k = " << phi_k_plain << "\n";
    std::cout << "  inv_φ^k = " << inv_phi_k_plain << "\n\n";

    // Encrypted constants
    std::vector<int64_t> vec_phi_k(ring_dim, 0);
    vec_phi_k[0] = NTL::to_long(phi_k_plain);

    std::vector<int64_t> vec_inv_phi_k(ring_dim, 0);
    vec_inv_phi_k[0] = NTL::to_long(inv_phi_k_plain);

    std::vector<int64_t> vec_zero(ring_dim, 0);
    std::vector<int64_t> vec_one(ring_dim, 0);
    vec_one[0] = NTL::to_long(phi_k_plain);

    auto ct_phi_k = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vec_phi_k));
    auto ct_inv_phi_k = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vec_inv_phi_k));
    auto ct_zero = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vec_zero));
    auto ct_one = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vec_one));

    // φ-NAND: φ^k - (a·b)·φ^(-k)
    auto homomorphic_nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto prod = cc->EvalMult(a, b);
        auto scaled = cc->EvalMult(prod, ct_inv_phi_k);
        return cc->EvalSub(ct_phi_k, scaled);
    };

    auto decrypt_raw = [&](Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };

    auto decrypt_to_bool = [&](Ciphertext<DCRTPoly> ct) {
        long long val = decrypt_raw(ct);
        long long target = NTL::to_long(phi_k_plain);
        return (val == target) ? 1 : 0;
    };

    std::cout << "φ-NAND TRUTH TABLE:\n";
    std::cout << "=====================\n\n";

    std::cout << "  NAND(0,0): raw=" << decrypt_raw(homomorphic_nand(ct_zero, ct_zero))
              << " (expected " << phi_k_plain << ", bool=" << decrypt_to_bool(homomorphic_nand(ct_zero, ct_zero)) << ")\n";
    std::cout << "  NAND(0,1): raw=" << decrypt_raw(homomorphic_nand(ct_zero, ct_one))
              << " (expected " << phi_k_plain << ", bool=" << decrypt_to_bool(homomorphic_nand(ct_zero, ct_one)) << ")\n";
    std::cout << "  NAND(1,0): raw=" << decrypt_raw(homomorphic_nand(ct_one, ct_zero))
              << " (expected " << phi_k_plain << ", bool=" << decrypt_to_bool(homomorphic_nand(ct_one, ct_zero)) << ")\n";
    std::cout << "  NAND(1,1): raw=" << decrypt_raw(homomorphic_nand(ct_one, ct_one))
              << " (expected 0, bool=" << decrypt_to_bool(homomorphic_nand(ct_one, ct_one)) << ")\n\n";

    // DEEP CHAIN
    std::cout << "DEEP CHAIN (10 gates):\n";
    std::cout << "=======================\n\n";

    auto current = ct_one;
    int errors = 0;

    for (int gate = 0; gate < 10; gate++) {
        current = homomorphic_nand(current, current);
        int got = decrypt_to_bool(current);
        int expected = (gate % 2 == 0) ? 0 : 1;

        if (got != expected) errors++;

        std::cout << "  Gate " << gate << ": expected=" << expected
                  << " got=" << got
                  << (expected == got ? " YES" : " NO")
                  << " raw=" << decrypt_raw(current) << "\n";
    }

    std::cout << "\n  Result: " << errors << "/10 errors ("
              << (100.0 * (10 - errors) / 10) << "%)\n";

    return 0;
}
