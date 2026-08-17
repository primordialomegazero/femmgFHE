// FULL ADDER WITH SIMD — 16,384 additions simultaneously!
#include "openfhe.h"
#include <iostream>
#include <vector>
#include <random>

using namespace lbcrypto;

int main() {
    std::cout << "====================================\n";
    std::cout << "  SIMD FULL ADDER (16384 at a time) \n";
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

    // NAND, NOT, AND, OR, XOR from NAND
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

    auto FULL_ADDER = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b, Ciphertext<DCRTPoly> c) {
        auto xor_ab = XOR(a, b);
        auto sum = XOR(xor_ab, c);
        auto and_ab = AND(a, b);
        auto and_xor_c = AND(xor_ab, c);
        auto carry = OR(and_ab, and_xor_c);
        return std::make_pair(sum, carry);
    };

    auto decrypt_vec = [&](Ciphertext<DCRTPoly> ct) -> std::vector<int64_t> {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue();
    };

    // Generate random bits for testing
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1);

    std::vector<int64_t> a_bits(num_slots), b_bits(num_slots), c_bits(num_slots);
    for (size_t i = 0; i < num_slots; i++) {
        a_bits[i] = dis(gen);
        b_bits[i] = dis(gen);
        c_bits[i] = dis(gen);
    }

    auto pt_a = cc->MakePackedPlaintext(a_bits);
    auto pt_b = cc->MakePackedPlaintext(b_bits);
    auto pt_c = cc->MakePackedPlaintext(c_bits);

    auto ct_a = cc->Encrypt(keys.publicKey, pt_a);
    auto ct_b = cc->Encrypt(keys.publicKey, pt_b);
    auto ct_c = cc->Encrypt(keys.publicKey, pt_c);

    std::cout << "Testing " << num_slots << " full adders simultaneously...\n";
    auto [ct_sum, ct_carry] = FULL_ADDER(ct_a, ct_b, ct_c);

    auto sum_vec = decrypt_vec(ct_sum);
    auto carry_vec = decrypt_vec(ct_carry);

    // Verify
    int errors = 0;
    for (size_t i = 0; i < num_slots; i++) {
        int expected_sum = a_bits[i] ^ b_bits[i] ^ c_bits[i];
        int expected_carry = (a_bits[i] & b_bits[i]) | (b_bits[i] & c_bits[i]) | (a_bits[i] & c_bits[i]);
        if (sum_vec[i] != expected_sum || carry_vec[i] != expected_carry) {
            errors++;
            if (errors < 5) {
                std::cout << "  ❌ Slot " << i << ": " << a_bits[i] << "+" << b_bits[i] << "+" << c_bits[i]
                          << " = " << carry_vec[i] << sum_vec[i] << " (exp " << expected_carry << expected_sum << ")\n";
            }
        }
    }

    std::cout << "\n--- RESULTS ---\n";
    std::cout << "  Total adders: " << num_slots << "\n";
    std::cout << "  Errors: " << errors << "/" << num_slots << "\n";

    if (errors == 0) {
        std::cout << "  ✅ ALL " << num_slots << " FULL ADDERS CORRECT!\n";
        std::cout << "  ✅ SIMD FULL ADDER WORKS!\n";
        std::cout << "  ✅ PERIOD-2 HOLDS FOR COMPLEX CIRCUITS!\n";
    }

    return 0;
}
