// 4-BIT MULTIPLIER WITH SIMD — Optimized for depth
#include "openfhe.h"
#include <iostream>
#include <vector>
#include <random>
#include <chrono>

using namespace lbcrypto;

int main() {
    std::cout << "====================================\n";
    std::cout << "  SIMD 4-BIT MULTIPLIER (16384 at a time) \n";
    std::cout << "====================================\n\n";

    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetPlaintextModulus(65537);
    parameters.SetMultiplicativeDepth(20);
    parameters.SetRingDim(32768);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    std::cout << "Generating keys...\n";
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    std::cout << "Keys ready!\n\n";

    size_t num_slots = 4096;  // Reduced para mas kontrolado ang noise
    std::vector<int64_t> ones_vec(num_slots, 1);
    auto pt_one = cc->MakePackedPlaintext(ones_vec);
    auto ct_one = cc->Encrypt(keys.publicKey, pt_one);
    std::vector<int64_t> zeros_vec(num_slots, 0);
    auto pt_zero = cc->MakePackedPlaintext(zeros_vec);
    auto ct_zero = cc->Encrypt(keys.publicKey, pt_zero);

    // Basic gates from NAND
    auto nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto ab = cc->EvalMult(a, b);
        return cc->EvalSub(ct_one, ab);
    };
    auto NOT = [&](Ciphertext<DCRTPoly> a) { return nand(a, a); };
    auto AND = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) { return NOT(nand(a, b)); };
    auto XOR = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto n1 = nand(a, b);
        return nand(nand(a, n1), nand(b, n1));
    };

    // 4-bit multiplier (optimized — partial products lang)
    auto multiply_4bit = [&](std::vector<Ciphertext<DCRTPoly>> A, std::vector<Ciphertext<DCRTPoly>> B) {
        std::vector<Ciphertext<DCRTPoly>> result(8, ct_zero);
        
        // Compute partial products
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                auto partial = AND(A[j], B[i]);
                
                // Add partial to result[i+j]
                // Simple XOR for now (no carry — test muna kung tama)
                result[i+j] = XOR(result[i+j], partial);
            }
        }
        
        return result;
    };

    std::cout << "Generating 4-bit test vectors...\n";
    std::mt19937_64 rng(42);
    std::vector<std::vector<int>> test_A(16384, std::vector<int>(4));
    std::vector<std::vector<int>> test_B(16384, std::vector<int>(4));
    
    for (int slot = 0; slot < 16384; slot++) {
        int a_val = rng() % 16;  // 0-15
        int b_val = rng() % 16;  // 0-15
        for (int bit = 0; bit < 4; bit++) {
            test_A[slot][bit] = (a_val >> bit) & 1;
            test_B[slot][bit] = (b_val >> bit) & 1;
        }
    }

    std::cout << "Encrypting...\n";
    std::vector<Ciphertext<DCRTPoly>> enc_A(4), enc_B(4);
    
    for (int bit = 0; bit < 4; bit++) {
        std::vector<int64_t> vec_A(num_slots), vec_B(num_slots);
        for (int slot = 0; slot < 16384; slot++) {
            vec_A[slot] = test_A[slot][bit];
            vec_B[slot] = test_B[slot][bit];
        }
        auto pt_A = cc->MakePackedPlaintext(vec_A);
        auto pt_B = cc->MakePackedPlaintext(vec_B);
        enc_A[bit] = cc->Encrypt(keys.publicKey, pt_A);
        enc_B[bit] = cc->Encrypt(keys.publicKey, pt_B);
    }

    std::cout << "Multiplying...\n";
    auto start = std::chrono::high_resolution_clock::now();
    auto result = multiply_4bit(enc_A, enc_B);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "Verifying...\n";
    int errors = 0;
    
    for (int bit = 0; bit < 8; bit++) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, result[bit], &pt);
        auto decrypted = pt->GetPackedValue();
        
        for (int slot = 0; slot < 16384; slot++) {
            int a_val = 0, b_val = 0;
            for (int i = 0; i < 4; i++) {
                a_val |= (test_A[slot][i] << i);
                b_val |= (test_B[slot][i] << i);
            }
            int expected = (a_val * b_val >> bit) & 1;
            if (decrypted[slot] != expected) {
                errors++;
                if (errors < 5) {
                    std::cout << "  Error at bit " << bit << ", slot " << slot 
                              << ": got " << decrypted[slot] 
                              << ", expected " << expected << "\n";
                }
            }
        }
    }

    std::cout << "\n--- RESULTS ---\n";
    std::cout << "  Total multipliers: 16384\n";
    std::cout << "  Errors: " << errors << "/" << (16384 * 8) << "\n";
    std::cout << "  Duration: " << duration << " ms\n";
    std::cout << "  Throughput: " << (16384.0 / (duration / 1000.0)) << " mult/sec\n";
    
    if (errors == 0) {
        std::cout << "\n  ✅ ALL 16,384 4-BIT MULTIPLIERS CORRECT!\n";
    } else {
        std::cout << "\n  ❌ ERRORS: " << errors << "\n";
    }

    return 0;
}
