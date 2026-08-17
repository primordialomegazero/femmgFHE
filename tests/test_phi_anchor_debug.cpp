// φ-ANCHOR NAND DEBUG
// Step-by-step para makita ang error pattern

#include "openfhe.h"
#include <NTL/ZZ.h>
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "φ-ANCHOR NAND DEBUG\n";
    std::cout << "===================\n\n";

    const int64_t PMOD = 3604481;
    
    NTL::ZZ p = NTL::to_ZZ(PMOD);
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), p);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), p);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % p;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + p) % p;
    NTL::ZZ inv_phi = NTL::InvMod(phi, p);

    int64_t phi_val = NTL::conv<int64_t>(phi);
    int64_t inv_phi_val = NTL::conv<int64_t>(inv_phi);

    // Scalar check muna — wala munang BFV
    std::cout << "1. SCALAR CHECK (walang BFV):\n";
    NTL::ZZ current = phi;
    
    for (int i = 0; i <= 5; i++) {
        NTL::ZZ expected = (i % 2 == 0) ? phi : NTL::to_ZZ(0);
        std::cout << "   Depth " << i << ": " << current << " (exp " << expected << ") ";
        std::cout << (current == expected ? "✓" : "✗") << "\n";
        
        // NAND: φ - current²·φ⁻¹
        NTL::ZZ sq = (current * current) % p;
        NTL::ZZ scaled = (sq * inv_phi) % p;
        NTL::ZZ nand = (phi - scaled) % p;
        if (nand < 0) nand += p;
        current = nand;
    }

    std::cout << "\n2. BFV SETUP:\n";
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
    auto ct_inv_phi = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({inv_phi_val}));

    auto decrypt_val = [&](Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        int64_t val = (int64_t)pt->GetPackedValue()[0];
        if (val < 0) val += PMOD;
        return val;
    };

    std::cout << "3. BFV DEEP CHAIN (10 depths):\n";
    auto ct_current = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({phi_val}));
    
    for (int i = 0; i <= 10; i++) {
        int64_t val = decrypt_val(ct_current);
        int64_t expected = (i % 2 == 0) ? phi_val : 0;
        std::cout << "   Depth " << i << ": " << val << " (exp " << expected << ") ";
        std::cout << (val == expected ? "✓" : "✗") << "\n";

        auto sq = cc->EvalMult(ct_current, ct_current);
        auto scaled = cc->EvalMult(sq, ct_inv_phi);
        ct_current = cc->EvalSub(ct_phi, scaled);
    }

    return 0;
}
