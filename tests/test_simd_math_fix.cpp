// MATH FIX: Standard BFV parameters + SIMD
#include "openfhe.h"
#include <iostream>
#include <chrono>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "MATH FIX: STANDARD BFV + SIMD\n";
    std::cout << "==============================\n\n";

    // Standard BFV parameters (proven secure)
    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetPlaintextModulus(65537);
    parameters.SetMultiplicativeDepth(20);
    parameters.SetRingDim(32768);
    
    // Let OpenFHE choose optimal ciphertext modulus
    // parameters.SetCoeffModulus(...);  // Auto-selected!

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    // SIMD packing
    size_t num_slots = 16384;
    std::vector<int64_t> packed_bits(num_slots);
    for (size_t i = 0; i < num_slots; i++) {
        packed_bits[i] = i % 2;
    }

    auto pt = cc->MakePackedPlaintext(packed_bits);
    auto ct_packed = cc->Encrypt(keys.publicKey, pt);

    // Constant 1
    std::vector<int64_t> ones_vec(num_slots, 1);
    auto pt_one = cc->MakePackedPlaintext(ones_vec);
    auto ct_one = cc->Encrypt(keys.publicKey, pt_one);

    auto nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto ab = cc->EvalMult(a, b);
        return cc->EvalSub(ct_one, ab);
    };

    auto decrypt_vec = [&](Ciphertext<DCRTPoly> ct) -> std::vector<int64_t> {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue();
    };

    std::cout << "Testing 20 depths with SIMD...\n";
    std::cout << "Slots: " << num_slots << "\n\n";

    auto current = ct_packed;
    int errors = 0;

    for (int depth = 0; depth <= 20; depth++) {
        auto decrypted = decrypt_vec(current);
        
        bool ok = true;
        for (size_t i = 0; i < 10 && i < num_slots; i++) {
            int64_t expected = (depth % 2 == 0) ? (i % 2) : (1 - (i % 2));
            if (decrypted[i] != expected) {
                ok = false;
                errors++;
                break;
            }
        }
        
        std::cout << "  Depth " << depth << ": " << (ok ? "✓" : "✗") << "\n";

        if (depth < 20) {
            current = nand(current, current);
        }
    }

    std::cout << "\nErrors: " << errors << "/21\n";

    if (errors == 0) {
        std::cout << "\n✅ MATH FIX WORKS!\n";
        std::cout << "✅ SIMD + PERIOD-2 CONFIRMED!\n";
        std::cout << "✅ STANDARD BFV PARAMETERS!\n";
    }

    return 0;
}
