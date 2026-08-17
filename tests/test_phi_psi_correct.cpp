// CORRECT φ-ψ ENCODING
// Message 0 = ψ = 1-φ, Message 1 = φ
// NAND(a,b) = φ - a·b·φ⁻¹

#include "openfhe.h"
#include <NTL/ZZ.h>
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "CORRECT φ-ψ ENCODING TEST\n";
    std::cout << "=========================\n\n";

    const int64_t PMOD = 3604481;
    
    NTL::ZZ p = NTL::to_ZZ(PMOD);
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), p);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), p);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % p;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + p) % p;
    NTL::ZZ inv_phi = NTL::InvMod(phi, p);

    int64_t phi_val = NTL::conv<int64_t>(phi);
    int64_t psi_val = NTL::conv<int64_t>(psi);
    int64_t inv_phi_val = NTL::conv<int64_t>(inv_phi);

    std::cout << "φ = " << phi_val << "\n";
    std::cout << "ψ = " << psi_val << "\n";
    std::cout << "φ⁻¹ = " << inv_phi_val << "\n\n";

    // BFV setup
    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetPlaintextModulus(PMOD);
    parameters.SetMultiplicativeDepth(10);
    parameters.SetRingDim(32768);
    
    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    // ENCODE: Message 0 = ψ, Message 1 = φ
    auto ct_zero = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({psi_val}));
    auto ct_one = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({phi_val}));
    auto ct_phi = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({phi_val}));
    auto ct_inv_phi = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({inv_phi_val}));

    auto decrypt_val = [&](Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        int64_t val = (int64_t)pt->GetPackedValue()[0];
        if (val < 0) val += PMOD;
        return val;
    };

    std::cout << "ENCODING:\n";
    std::cout << "  Message 0 (ψ) = " << decrypt_val(ct_zero) << "\n";
    std::cout << "  Message 1 (φ) = " << decrypt_val(ct_one) << "\n\n";

    // NAND function: φ - a·b·φ⁻¹
    auto hom_nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto ab = cc->EvalMult(a, b);
        auto ab_scaled = cc->EvalMult(ab, ct_inv_phi);
        return cc->EvalSub(ct_phi, ab_scaled);
    };

    std::cout << "NAND TESTS:\n";
    auto nand_00 = hom_nand(ct_zero, ct_zero);
    std::cout << "  NAND(ψ,ψ) = " << decrypt_val(nand_00) << " (exp φ=" << phi_val << ") ";
    std::cout << (decrypt_val(nand_00) == phi_val ? "✓" : "✗") << "\n";

    auto nand_01 = hom_nand(ct_zero, ct_one);
    std::cout << "  NAND(ψ,φ) = " << decrypt_val(nand_01) << " (exp φ=" << phi_val << ") ";
    std::cout << (decrypt_val(nand_01) == phi_val ? "✓" : "✗") << "\n";

    auto nand_10 = hom_nand(ct_one, ct_zero);
    std::cout << "  NAND(φ,ψ) = " << decrypt_val(nand_10) << " (exp φ=" << phi_val << ") ";
    std::cout << (decrypt_val(nand_10) == phi_val ? "✓" : "✗") << "\n";

    auto nand_11 = hom_nand(ct_one, ct_one);
    std::cout << "  NAND(φ,φ) = " << decrypt_val(nand_11) << " (exp ψ=" << psi_val << ") ";
    std::cout << (decrypt_val(nand_11) == psi_val ? "✓" : "✗") << "\n\n";

    // Deep chain
    std::cout << "DEEP CHAIN (20 depths):\n";
    auto current = ct_one;
    int errors = 0;

    for (int i = 0; i <= 20; i++) {
        int64_t val = decrypt_val(current);
        int64_t expected = (i % 2 == 0) ? phi_val : psi_val;
        bool match = (val == expected);
        if (!match) errors++;

        auto sq = cc->EvalMult(current, current);
        auto sq_scaled = cc->EvalMult(sq, ct_inv_phi);
        current = cc->EvalSub(ct_phi, sq_scaled);
    }

    std::cout << "  Errors: " << errors << "/21\n";
    std::cout << "\nRESULT: " << (errors == 0 ? "PERFECT PERIOD-2! ✓" : "May errors") << "\n";

    return 0;
}
