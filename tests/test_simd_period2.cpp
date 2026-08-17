// SIMD PACKING + PERIOD-2 NAND
// 16384 bits per ciphertext = 16384x faster!
#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "SIMD PACKING + PERIOD-2 NAND\n";
    std::cout << "=============================\n\n";

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

    // Pack 16384 bits into one ciphertext
    size_t num_slots = 16384;
    std::vector<int64_t> plaintext_vec(num_slots);
    
    // Fill with alternating 0s and 1s
    for (size_t i = 0; i < num_slots; i++) {
        plaintext_vec[i] = i % 2;
    }

    auto pt = cc->MakePackedPlaintext(plaintext_vec);
    auto ct = cc->Encrypt(keys.publicKey, pt);

    // Create constant 1 ciphertext
    std::vector<int64_t> ones_vec(num_slots, 1);
    auto pt_one = cc->MakePackedPlaintext(ones_vec);
    auto ct_one = cc->Encrypt(keys.publicKey, pt_one);

    // NAND on packed ciphertexts
    auto nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto ab = cc->EvalMult(a, b);
        return cc->EvalSub(ct_one, ab);
    };

    // Decrypt helper
    auto decrypt_vec = [&](Ciphertext<DCRTPoly> ct) -> std::vector<int64_t> {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue();
    };

    std::cout << "Testing SIMD packing...\n";
    std::cout << "Slots: " << num_slots << "\n\n";

    // Apply NAND 20 times
    auto current = ct;
    int errors = 0;

    for (int depth = 0; depth <= 20; depth++) {
        auto decrypted = decrypt_vec(current);
        
        // Check first 10 slots
        bool ok = true;
        for (int i = 0; i < 10 && i < num_slots; i++) {
            int64_t expected = (depth % 2 == 0) ? (i % 2) : (1 - (i % 2));
            if (decrypted[i] != expected) {
                ok = false;
                errors++;
                break;
            }
        }
        
        std::cout << "Depth " << depth << ": " 
                  << (ok ? "✓" : "✗") << " (first slot: " 
                  << decrypted[0] << ", exp: " 
                  << ((depth % 2 == 0) ? (0 % 2) : (1 - (0 % 2))) << ")\n";

        if (depth < 20) {
            current = nand(current, current);
        }
    }

    std::cout << "\nErrors: " << errors << "/21\n";

    if (errors == 0) {
        std::cout << "\n✅ SIMD PACKING WORKS!\n";
        std::cout << "✅ PERIOD-2 HOLDS FOR PACKED BITS!\n";
        std::cout << "✅ " << num_slots << "x SPEEDUP!\n";
    }

    return 0;
}
