// FIXED: Tamang NAND formula sa φ-ψ encoding
#include "openfhe.h"
#include <NTL/ZZ.h>
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "FIXED: φ-ψ ENCODING WITH CORRECT NAND\n";
    std::cout << "======================================\n\n";

    const int64_t PMOD = 3604481;

    NTL::ZZ p = NTL::to_ZZ(PMOD);
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), p);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), p);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % p;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + p) % p;

    int64_t phi_val = NTL::conv<int64_t>(phi);
    int64_t psi_val = NTL::conv<int64_t>(psi);

    std::cout << "φ = " << phi_val << " (message 1)\n";
    std::cout << "ψ = " << psi_val << " (message 0)\n\n";

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

    auto ct_phi = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({phi_val}));
    auto ct_psi = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({psi_val}));

    // TAMANG NAND: φ - a·b
    auto nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto ab = cc->EvalMult(a, b);
        return cc->EvalSub(ct_phi, ab);
    };

    // Test NAND(φ,φ) = ψ
    auto nand_phi_phi = nand(ct_phi, ct_phi);
    Plaintext pt;
    cc->Decrypt(keys.secretKey, nand_phi_phi, &pt);
    int64_t result = pt->GetPackedValue()[0];
    std::cout << "NAND(φ,φ) = " << result << " (exp " << psi_val << ") "
              << (result == psi_val ? "✓" : "✗") << "\n";

    // Test NAND(ψ,ψ) = φ
    auto nand_psi_psi = nand(ct_psi, ct_psi);
    cc->Decrypt(keys.secretKey, nand_psi_psi, &pt);
    result = pt->GetPackedValue()[0];
    std::cout << "NAND(ψ,ψ) = " << result << " (exp " << phi_val << ") "
              << (result == phi_val ? "✓" : "✗") << "\n";

    // Test NAND(φ,ψ) = φ
    auto nand_phi_psi = nand(ct_phi, ct_psi);
    cc->Decrypt(keys.secretKey, nand_phi_psi, &pt);
    result = pt->GetPackedValue()[0];
    std::cout << "NAND(φ,ψ) = " << result << " (exp " << phi_val << ") "
              << (result == phi_val ? "✓" : "✗") << "\n";

    // Test NAND(ψ,φ) = φ
    auto nand_psi_phi = nand(ct_psi, ct_phi);
    cc->Decrypt(keys.secretKey, nand_psi_phi, &pt);
    result = pt->GetPackedValue()[0];
    std::cout << "NAND(ψ,φ) = " << result << " (exp " << phi_val << ") "
              << (result == phi_val ? "✓" : "✗") << "\n";

    // Deep chain test
    std::cout << "\nDEEP CHAIN (10 depths):\n";
    auto current = ct_phi;
    int errors = 0;
    for (int i = 0; i <= 10; i++) {
        cc->Decrypt(keys.secretKey, current, &pt);
        int64_t val = pt->GetPackedValue()[0];
        int64_t expected = (i % 2 == 0) ? phi_val : psi_val;
        bool ok = (val == expected);
        if (!ok) errors++;
        std::cout << "  Depth " << i << ": " << val << " (exp " << expected << ") "
                  << (ok ? "✓" : "✗") << "\n";
        if (i < 10) {
            current = nand(current, current);
        }
    }
    std::cout << "Errors: " << errors << "/11\n";

    if (errors == 0) {
        std::cout << "\n✅ PERIOD-2 CONFIRMED WITH φ-ψ ENCODING!\n";
        std::cout << "✅ NATURAL BOOTSTRAPPING WORKS!\n";
    } else {
        std::cout << "\n⚠️ Errors detected. Check encoding.\n";
    }

    return 0;
}
