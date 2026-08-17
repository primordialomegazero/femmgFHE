// BFV PERIOD-2 — Using BFV para sa tunay na homomorphic NAND
#include "openfhe.h"
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "BFV PERIOD-2 NAND TEST\n";
    std::cout << "======================\n\n";
    
    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetPlaintextModulus(65537);
    parameters.SetMultiplicativeDepth(20);
    parameters.SetRingDim(32768);
    
    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    
    // φ = L(42) = 599074578 mod 65537
    int64_t phi = 599074578LL % 65537;
    int64_t inv_phi = 1; // Kailangan i-compute ang inverse
    for (int64_t i = 1; i < 65537; i++) {
        if ((phi * i) % 65537 == 1) {
            inv_phi = i;
            break;
        }
    }
    
    std::cout << "φ (mod 65537) = " << phi << "\n";
    std::cout << "φ⁻¹ = " << inv_phi << "\n\n";
    
    // Encrypt 0 at φ
    auto ct0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({0}));
    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({phi}));
    
    // Period-2 NAND: φ - x²·φ⁻¹
    auto nand = [&](Ciphertext<DCRTPoly> x) {
        auto sq = cc->EvalMult(x, x);
        auto sq_inv = cc->EvalMult(sq, cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({inv_phi})));
        return cc->EvalSub(ct1, sq_inv);
    };
    
    // Deep chain
    auto current = ct1;
    int errors = 0;
    std::cout << "Deep chain (50 depths):\n";
    for (int i = 0; i <= 20; i++) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, current, &pt);
        int64_t val = pt->GetPackedValue()[0];
        int expected = (i % 2 == 0) ? (int)phi : 0;
        
        if (val != expected) {
            if (errors < 5) std::cout << "  Depth " << i << ": " << val << " (exp " << expected << ") ✗\n";
            errors++;
        }
        
        current = nand(current);
    }
    
    std::cout << "Errors: " << errors << "/21\n";
    return 0;
}
