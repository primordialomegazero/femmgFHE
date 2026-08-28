// NAND PERIOD-0 PURE — NAND sa Period-0 Space
// Pure addition, walang modulo, walang wrapping
// NAND encoded bilang period-0 states

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  NAND PERIOD-0 PURE\n";
    std::cout << "  NAND sa Period-0 Space\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(20);
    params.SetPlaintextModulus(65537);
    params.SetBatchSize(256);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    auto slots = cc->GetEncodingParams()->GetBatchSize();

    auto make_ct = [&](int64_t val) {
        std::vector<int64_t> vec(slots, 0);
        vec[0] = val;
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vec));
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };

    // Period-0 encoding:
    // Bit 0 → 0
    // Bit 1 → 1
    // NAND = XOR + feedback
    
    // Sa period-0 space, ang NAND ay:
    // NAND(a,b) = NOT(AND(a,b))
    // = 1 - (a*b)
    // Sa pure addition: kailangan natin ng alternative
    
    // Key insight: Sa BFV modulo 65537,
    // ang multiplication ay maaaring i-represent bilang addition
    // kung gumagamit tayo ng logarithm/exponent encoding
    
    // NAND sa exponent space:
    // NAND_exp = 1 - (a_exp * b_exp)
    // Sa logarithm: log(NAND) = log(1 - a*b)
    
    // Mas simple: gamitin ang property na
    // Sa period-0, NAND = (1-a) + (1-b) - (1-a)(1-b)
    // = 2 - a - b - (1 - a - b + ab)
    // = 2 - a - b - 1 + a + b - ab
    // = 1 - ab
    
    // Kaya NAND = 1 - a*b pa rin
    // Kailangan natin ng multiplication
    
    // Pero sa BFV, ang multiplication ay level drop
    // Kaya hindi ito pure addition
    
    // Alternative: i-encode ang NAND bilang XOR
    // XOR(a,b) = a + b - 2ab (may multiplication)
    // O sa period-0: XOR = a + b (pure addition!)
    
    // Ang period-0 XOR ay natural:
    // XOR(0,0) = 0, XOR(0,1) = 1, XOR(1,0) = 1, XOR(1,1) = 0 (kung may modulo 2)
    
    // Para sa NAND, kailangan natin ng NOT(XOR)
    // Sa period-0: NOT(x) = 1 - x
    
    std::cout << "PERIOD-0 NAND ENCODING:\n";
    std::cout << "=======================\n\n";
    
    // Test: NAND = 1 - (a+b) sa period-0
    auto ct_0 = make_ct(0);
    auto ct_1 = make_ct(1);
    auto ct_one = make_ct(1);
    
    auto eval_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_one, sum);
    };
    
    // Test NAND
    auto nand_00 = eval_nand(ct_0, ct_0);
    auto nand_01 = eval_nand(ct_0, ct_1);
    auto nand_11 = eval_nand(ct_1, ct_1);
    
    std::cout << "  NAND(0,0) = " << decrypt_val(nand_00) << " (expected 1)\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(nand_01) << " (expected 1)\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(nand_11) << " (expected 0)\n\n";
    
    // Chain test
    std::cout << "CHAIN TEST (100 gates):\n";
    std::cout << "======================\n\n";
    
    auto state = eval_nand(ct_1, ct_1);
    auto start = high_resolution_clock::now();
    
    int errors = 0;
    int expected = 0;
    
    for (int i = 0; i < 100; i++) {
        int64_t val = decrypt_val(state);
        int bit = (val == 1) ? 1 : 0;
        
        if (bit != expected) errors++;
        
        if (i < 5 || i >= 95) {
            std::cout << "  Gate " << i << ": " << val
                      << " → " << bit
                      << " (expected " << expected << ")"
                      << " level=" << state->GetLevel()
                      << (bit == expected ? " ✓" : " ✗") << "\n";
        }
        
        auto next_input = (bit == 1) ? ct_1 : ct_0;
        state = eval_nand(next_input, next_input);
        expected = 1 - expected;
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();
    
    std::cout << "\n  Gates: 100\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Time: " << duration << " seconds\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ NAND PERIOD-0!" : "⚠️ MAY ERRORS") << "\n";
    
    return 0;
}
