// φ-ψ UNIVERSAL ENCODING SA BFV
// Message 0 = ψ, Message 1 = φ
// NAND(φ,φ) = ψ, NAND(ψ,ψ) = φ — PERFECT PERIOD-2

#include "openfhe.h"
#include <NTL/ZZ.h>
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "φ-ψ UNIVERSAL ENCODING SA BFV\n";
    std::cout << "=============================\n\n";

    // Compute φ at ψ (scalar values)
    NTL::ZZ Q_phi = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q_phi);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q_phi);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q_phi;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q_phi) % Q_phi;

    // I-mod sa BFV plaintext modulus (65537)
    int64_t phi_mod = NTL::conv<int64_t>(phi % 3604481);
    int64_t psi_mod = NTL::conv<int64_t>(psi % 3604481);

    std::cout << "φ mod 3604481 = " << phi_mod << "\n";
    std::cout << "ψ mod 65537 = " << psi_mod << "\n\n";

    // BFV setup
    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetPlaintextModulus(3604481);  // Compatible prime!
    parameters.SetMultiplicativeDepth(10);
    parameters.SetRingDim(32768);
    
    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    std::cout << "1. BFV SETUP DONE\n\n";

    // Encrypt φ at ψ
    auto ct_phi = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({phi_mod}));
    auto ct_psi = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({psi_mod}));
    auto ct_one = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({1}));

    // Decrypt helper
    auto decrypt_val = [&](Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return (int64_t)pt->GetPackedValue()[0];
    };

    std::cout << "2. VERIFY ENCODING:\n";
    std::cout << "   Decrypt(φ) = " << decrypt_val(ct_phi) << " (exp " << phi_mod << ")\n";
    std::cout << "   Decrypt(ψ) = " << decrypt_val(ct_psi) << " (exp " << psi_mod << ")\n\n";

    // NAND(φ,φ) = 1 - φ² = ψ
    std::cout << "3. PERIOD-2 NAND:\n";
    
    auto sq_phi = cc->EvalMult(ct_phi, ct_phi);
    auto nand_phi_phi = cc->EvalSub(ct_one, sq_phi);
    int64_t nand_phi_result = decrypt_val(nand_phi_phi);
    std::cout << "   NAND(φ,φ) = " << nand_phi_result << " (exp " << psi_mod << ")\n";
    std::cout << "   Match: " << (nand_phi_result == psi_mod ? "YES ✓" : "NO ✗") << "\n\n";

    auto sq_psi = cc->EvalMult(ct_psi, ct_psi);
    auto nand_psi_psi = cc->EvalSub(ct_one, sq_psi);
    int64_t nand_psi_result = decrypt_val(nand_psi_psi);
    std::cout << "   NAND(ψ,ψ) = " << nand_psi_result << " (exp " << phi_mod << ")\n";
    std::cout << "   Match: " << (nand_psi_result == phi_mod ? "YES ✓" : "NO ✗") << "\n\n";

    // Deep chain (50 depths)
    std::cout << "4. DEEP CHAIN (50 depths):\n";
    auto current = ct_phi;
    int errors = 0;
    
    std::cout << "   Depth | Value | Expected | Status\n";
    std::cout << "   ------|-------|----------|--------\n";

    for (int i = 0; i <= 50; i++) {
        int64_t val = decrypt_val(current);
        int64_t expected = (i % 2 == 0) ? phi_mod : psi_mod;
        bool match = (val == expected);
        if (!match) errors++;

        if (i <= 10 || i % 10 == 0) {
            std::cout << "   " << i << " | " << val << " | " << expected 
                      << " | " << (match ? "✓" : "✗") << "\n";
        }

        auto sq = cc->EvalMult(current, current);
        current = cc->EvalSub(ct_one, sq);
    }

    std::cout << "\n   Errors: " << errors << "/51\n\n";

    std::cout << "5. RESULT: " << (errors == 0 ? "PERFECT ✓" : "May errors") << "\n";

    return 0;
}
