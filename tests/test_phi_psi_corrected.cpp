// CORRECTED: Tamang φ-ψ encoding na may modulo wrapping
#include "openfhe.h"
#include <NTL/ZZ.h>
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "CORRECTED: φ-ψ ENCODING\n";
    std::cout << "=======================\n\n";

    const int64_t PMOD = 3604481;

    // Compute φ and ψ modulo PMOD
    NTL::ZZ p = NTL::to_ZZ(PMOD);
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), p);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), p);
    NTL::ZZ phi_zz = ((NTL::to_ZZ(1) + sqrt5) * inv2) % p;
    NTL::ZZ psi_zz = (NTL::to_ZZ(1) - phi_zz + p) % p;

    int64_t phi_val = NTL::conv<int64_t>(phi_zz);
    int64_t psi_val = NTL::conv<int64_t>(psi_zz);

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

    // I-encrypt ang φ at ψ
    auto ct_phi = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({phi_val}));
    auto ct_psi = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({psi_val}));

    // TAMANG NAND: φ - a·b (with proper modulo)
    auto nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto ab = cc->EvalMult(a, b);
        return cc->EvalSub(ct_phi, ab);
    };

    // Helper para mag-decrypt at mag-wrap
    auto decrypt_wrap = [&](Ciphertext<DCRTPoly> ct) -> int64_t {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        int64_t val = pt->GetPackedValue()[0];
        // I-wrap sa [0, PMOD-1]
        val %= PMOD;
        if (val < 0) val += PMOD;
        return val;
    };

    // Test NAND gates
    std::cout << "NAND TESTS:\n";
    
    int64_t result = decrypt_wrap(nand(ct_phi, ct_phi));
    std::cout << "  NAND(φ,φ) = " << result << " (exp " << psi_val << ") "
              << (result == psi_val ? "✓" : "✗") << "\n";

    result = decrypt_wrap(nand(ct_psi, ct_psi));
    std::cout << "  NAND(ψ,ψ) = " << result << " (exp " << phi_val << ") "
              << (result == phi_val ? "✓" : "✗") << "\n";

    result = decrypt_wrap(nand(ct_phi, ct_psi));
    std::cout << "  NAND(φ,ψ) = " << result << " (exp " << phi_val << ") "
              << (result == phi_val ? "✓" : "✗") << "\n";

    result = decrypt_wrap(nand(ct_psi, ct_phi));
    std::cout << "  NAND(ψ,φ) = " << result << " (exp " << phi_val << ") "
              << (result == phi_val ? "✓" : "✗") << "\n";

    // Deep chain test
    std::cout << "\nDEEP CHAIN (10 depths):\n";
    auto current = ct_phi;
    int errors = 0;
    for (int i = 0; i <= 10; i++) {
        int64_t val = decrypt_wrap(current);
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
        std::cout << "\n✅ PERIOD-2 CONFIRMED!\n";
    } else {
        std::cout << "\n⚠️ May errors. Check parameters.\n";
    }

    return 0;
}
