// 8-BIT MULTIPLIER WITH SIMD — 16,384 multipliers simultaneously!
#include "openfhe.h"
#include <iostream>
#include <vector>
#include <random>
#include <chrono>

using namespace lbcrypto;

int main() {
    std::cout << "====================================\n";
    std::cout << "  SIMD 8-BIT MULTIPLIER (16384 at a time) \n";
    std::cout << "====================================\n\n";

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

    size_t num_slots = 16384;
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
    auto OR = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) { return nand(NOT(a), NOT(b)); };
    auto XOR = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto n1 = nand(a, b);
        return nand(nand(a, n1), nand(b, n1));
    };

    // Full adder
    auto full_adder = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b, Ciphertext<DCRTPoly> cin) {
        auto sum1 = XOR(a, b);
        auto sum = XOR(sum1, cin);
        auto carry1 = AND(a, b);
        auto carry2 = AND(sum1, cin);
        auto carry = OR(carry1, carry2);
        return std::make_pair(sum, carry);
    };

    // 8-bit multiplier (shift and add)
    auto multiply_8bit = [&](std::vector<Ciphertext<DCRTPoly>> A, std::vector<Ciphertext<DCRTPoly>> B) {
        std::vector<Ciphertext<DCRTPoly>> result(16, ct_zero);
        
        for (int i = 0; i < 8; i++) {
            Ciphertext<DCRTPoly> carry = ct_zero;
            for (int j = 0; j < 8; j++) {
                auto partial = AND(A[j], B[i]);
                auto sum = XOR(result[i+j], partial);
                auto carry1 = AND(result[i+j], partial);
                auto carry2 = AND(sum, carry);
                auto new_carry = OR(carry1, carry2);
                result[i+j] = XOR(sum, carry);
                carry = new_carry;
            }
            result[i+8] = carry;
        }
        
        return result;
    };

    std::cout << "Generating 8-bit test vectors...\n";
    
    // Random test vectors
    std::mt19937_64 rng(42);
    std::vector<std::vector<int>> test_A(16384, std::vector<int>(8));
    std::vector<std::vector<int>> test_B(16384, std::vector<int>(8));
    
    for (int slot = 0; slot < 16384; slot++) {
        int a_val = rng() % 256;
        int b_val = rng() % 256;
        for (int bit = 0; bit < 8; bit++) {
            test_A[slot][bit] = (a_val >> bit) & 1;
            test_B[slot][bit] = (b_val >> bit) & 1;
        }
    }

    std::cout << "Encrypting test vectors...\n";
    // FIX: Use vector<Ciphertext> not vector<vector<Ciphertext>>
    std::vector<Ciphertext<DCRTPoly>> enc_A(8), enc_B(8);
    
    for (int bit = 0; bit < 8; bit++) {
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

    std::cout << "Performing 8-bit multiplication...\n";
    auto start = std::chrono::high_resolution_clock::now();
    auto result = multiply_8bit(enc_A, enc_B);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "Decrypting and verifying...\n";
    int errors = 0;
    
    for (int bit = 0; bit < 16; bit++) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, result[bit], &pt);
        auto decrypted = pt->GetPackedValue();
        
        for (int slot = 0; slot < 16384; slot++) {
            int a_val = 0, b_val = 0;
            for (int i = 0; i < 8; i++) {
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
    std::cout << "  Total bits: " << (16384 * 16) << "\n";
    std::cout << "  Errors: " << errors << "/" << (16384 * 16) << "\n";
    std::cout << "  Duration: " << duration << " ms\n";
    std::cout << "  Throughput: " << (16384.0 / (duration / 1000.0)) << " multiplications/sec\n";
    
    if (errors == 0) {
        std::cout << "\n  ✅ ALL 16,384 8-BIT MULTIPLIERS CORRECT!\n";
        std::cout << "  ✅ SIMD 8-BIT MULTIPLIER WORKS!\n";
        std::cout << "  ✅ PERIOD-2 HOLDS FOR COMPLEX CIRCUITS!\n";
    } else {
        std::cout << "\n  ❌ ERRORS DETECTED: " << errors << "\n";
    }

    return 0;
}
