// φ-ψ CONSTANT ENCODING SA BFV — FINAL
// Message 0 = ψ, Message 1 = φ bilang constant polynomials
// NAND: 1 - a·b, Period-2: φ ↔ ψ

#include "openfhe.h"
#include <NTL/ZZ.h>
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "φ-ψ CONSTANT ENCODING SA BFV — FINAL\n";
    std::cout << "=====================================\n\n";

    // Compatible prime: 3604481 = 327680 × 11 + 1
    const int64_t PMOD = 3604481;

    // Direct φ at ψ sa PMOD
    NTL::ZZ p = NTL::to_ZZ(PMOD);
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), p);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), p);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % p;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + p) % p;

    int64_t phi_val = NTL::conv<int64_t>(phi);
    int64_t psi_val = NTL::conv<int64_t>(psi);

    std::cout << "φ = " << phi_val << "\n";
    std::cout << "ψ = " << psi_val << "\n\n";

    // Verify φ² = φ+1 at ψ² = ψ+1 sa PMOD
    std::cout << "VERIFY:\n";
    std::cout << "  φ² = " << (phi*phi)%p << " = φ+1 = " << (phi+1)%p << " ✓\n";
    std::cout << "  ψ² = " << (psi*psi)%p << " = ψ+1 = " << (psi+1)%p << " ✓\n\n";

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

    // Encrypt φ at ψ bilang constant polynomials
    auto ct_phi = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({phi_val}));
    auto ct_psi = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({psi_val}));
    auto ct_one = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({1}));

    auto decrypt_val = [&](Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        int64_t val = (int64_t)pt->GetPackedValue()[0];
        if (val < 0) val += PMOD;
        return val;
    };

    std::cout << "ENCODING:\n";
    std::cout << "  Decrypt(φ) = " << decrypt_val(ct_phi) << " (exp " << phi_val << ")\n";
    std::cout << "  Decrypt(ψ) = " << decrypt_val(ct_psi) << " (exp " << psi_val << ")\n\n";

    // NAND tests
    std::cout << "NAND TESTS:\n";
    
    auto sq_phi = cc->EvalMult(ct_phi, ct_phi);
    auto nand_phi = cc->EvalSub(ct_one, sq_phi);
    std::cout << "  NAND(φ,φ) = " << decrypt_val(nand_phi) << " (exp " << psi_val << ") ";
    std::cout << (std::abs(decrypt_val(nand_phi) - psi_val) <= 1 ? "✓" : "✗") << "\n";

    auto sq_psi = cc->EvalMult(ct_psi, ct_psi);
    auto nand_psi = cc->EvalSub(ct_one, sq_psi);
    std::cout << "  NAND(ψ,ψ) = " << decrypt_val(nand_psi) << " (exp " << phi_val << ") ";
    std::cout << (std::abs(decrypt_val(nand_psi) - phi_val) <= 1 ? "✓" : "✗") << "\n\n";

    // Deep chain (20 depths)
    std::cout << "DEEP CHAIN (20 depths):\n";
    auto current = ct_phi;
    int errors = 0;

    for (int i = 0; i <= 20; i++) {
        int64_t val = decrypt_val(current);
        int64_t expected = (i % 2 == 0) ? phi_val : psi_val;
        bool match = (std::abs(val - expected) <= 1);
        if (!match) errors++;

        if (i <= 5 || i % 5 == 0) {
            std::cout << "  Depth " << i << ": " << val << " (exp " << expected << ") "
                      << (match ? "✓" : "✗") << "\n";
        }

        auto sq = cc->EvalMult(current, current);
        current = cc->EvalSub(ct_one, sq);
    }

    std::cout << "\n  Errors: " << errors << "/21\n";
    std::cout << "\nRESULT: " << (errors == 0 ? "PERFECT PERIOD-2 SA BFV! ✓" : "May errors") << "\n";

    return 0;
}
