// OPENFHE φ-NAND FIXED — Proper Modulo Handling
#include <NTL/ZZ.h>
#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  OPENFHE φ-NAND FIXED\n";
    std::cout << "  Proper Modulo Handling\n";
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

    // φ-Structure
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
    std::cout << "  φ^k = " << phi_k_plain << " (or " << (phi_k_plain > p_mod/2 ? phi_k_plain - p_mod : phi_k_plain) << ")\n";
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

    // Normalize: i-interpret ang value sa [-p/2, p/2]
    auto normalize_val = [&](long long val) {
        long long p = 65537;
        if (val > p/2) val -= p;
        return val;
    };

    // Decision: ang φ^k ay kinakatawan ng value na nasa [0, p/2]
    // Pero sa normalization, ito ay negative value
    auto decrypt_to_bool = [&](Ciphertext<DCRTPoly> ct) {
        long long raw = decrypt_raw(ct);
        long long norm = normalize_val(raw);
        
        // Ang φ^k ay maaaring positive (60704) o negative (-4833)
        // Sa BFV, ito ay laging lumalabas bilang modulo 65537
        // Kaya: ang "1" ay raw == 60704 O norm == phi_k_plain as negative
        long long target_pos = NTL::to_long(phi_k_plain);
        long long target_neg = NTL::to_long(phi_k_plain) - 65537;
        
        return (raw == target_pos || norm == target_neg) ? 1 : 0;
    };

    std::cout << "φ-NAND TRUTH TABLE:\n";
    std::cout << "=====================\n\n";

    auto nand_00 = homomorphic_nand(ct_zero, ct_zero);
    auto nand_01 = homomorphic_nand(ct_zero, ct_one);
    auto nand_10 = homomorphic_nand(ct_one, ct_zero);
    auto nand_11 = homomorphic_nand(ct_one, ct_one);

    std::cout << "  NAND(0,0): raw=" << decrypt_raw(nand_00)
              << " norm=" << normalize_val(decrypt_raw(nand_00))
              << " bool=" << decrypt_to_bool(nand_00) << "\n";
    std::cout << "  NAND(0,1): raw=" << decrypt_raw(nand_01)
              << " norm=" << normalize_val(decrypt_raw(nand_01))
              << " bool=" << decrypt_to_bool(nand_01) << "\n";
    std::cout << "  NAND(1,0): raw=" << decrypt_raw(nand_10)
              << " norm=" << normalize_val(decrypt_raw(nand_10))
              << " bool=" << decrypt_to_bool(nand_10) << "\n";
    std::cout << "  NAND(1,1): raw=" << decrypt_raw(nand_11)
              << " norm=" << normalize_val(decrypt_raw(nand_11))
              << " bool=" << decrypt_to_bool(nand_11) << "\n\n";

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
                  << " raw=" << decrypt_raw(current)
                  << " norm=" << normalize_val(decrypt_raw(current)) << "\n";
    }

    std::cout << "\n  Result: " << errors << "/10 errors ("
              << (100.0 * (10 - errors) / 10) << "%)\n";

    return 0;
}
