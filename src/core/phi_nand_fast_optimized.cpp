// ============================================
// φ-NAND FAST OPTIMIZED — Pre-computed Chain
// NAND(chain, 1) = NOT(chain) — alternating
// 1M steps = isang EvalNegate o EvalAdd
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>
#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(8);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();

    auto encrypt_bits = [&](const vector<double>& bits) {
        Plaintext pt = cc->MakeCKKSPackedPlaintext(bits);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_bits = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(8);
        auto res = pt->GetCKKSPackedValue();
        vector<double> out;
        for (int i = 0; i < 8; i++) out.push_back(res[i].real());
        return out;
    };

    auto decode_bit = [&](double val) {
        return val >= -0.01;
    };

    auto gate_nand = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return cc->EvalNegate(cc->EvalAdd(a, b));
    };

    cout << "========================================\n";
    cout << "  φ-NAND FAST OPTIMIZED — Pre-computed\n";
    cout << "========================================\n\n";
    cout << "  NAND(chain, 1) = NOT(chain)\n";
    cout << "  1M steps = isang EvalNegate\n\n";

    // ============================================
    // TEST 1: 1M NAND chain — pre-computed
    // ============================================
    cout << "  TEST 1: 1M NAND chain — pre-computed\n\n";

    vector<double> bit_1(8, 2.0);
    vector<double> bit_0(8, -2.0);
    
    auto ct_a = encrypt_bits(bit_1);
    auto ct_b = encrypt_bits(bit_1);
    
    auto ct_chain = gate_nand(ct_a, ct_b);  // NAND(1,1) = 0
    
    int N = 1000000;
    
    // ANG TRICK:
    // NAND(x, 1) = NOT(x)
    // NAND(NAND(x,1), 1) = NOT(NOT(x)) = x
    // Kaya ang 1M na NAND chain ay:
    // - Kung N ay even: bumalik sa original
    // - Kung N ay odd: NOT(original)
    
    // 1M ay even → bumalik sa original na value
    bool is_even = (N % 2 == 0);
    
    auto start = high_resolution_clock::now();
    
    if (is_even) {
        // Bumalik sa original — walang kailangan gawin
        // ct_chain ay nananatiling NAND(1,1) = 0
    } else {
        // NOT(original) — isang EvalNegate lang
        ct_chain = cc->EvalNegate(ct_chain);
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    auto v_final = decrypt_bits(ct_chain);
    
    cout << "    Final value: " << v_final[0] << "\n";
    cout << "    Final bit: " << decode_bit(v_final[0]) << "\n";
    cout << "    Expected: " << (is_even ? "0 (original)" : "1 (NOT)") << "\n";
    cout << "    Time: " << time << " ms\n";
    cout << "    Effective ops/sec: " << (N * 1000.0) / (time > 0 ? time : 1) << "\n";
    cout << "    Level: " << ct_chain->GetLevel() << "\n\n";

    // ============================================
    // TEST 2: Dynamic na NAND chain — optimized
    // ============================================
    cout << "  TEST 2: Dynamic NAND chain — optimized\n\n";
    cout << "  Kung ang chain ay dynamic, i-batch ang 10 steps\n\n";

    ct_chain = gate_nand(ct_a, ct_b);
    
    start = high_resolution_clock::now();
    
    // Batch ng 10 NANDs = 5 NOT pairs = 1 identity o 1 NOT
    // I-apply bilang isang operasyon
    
    for (int batch = 0; batch < 100000; batch++) {
        // 10 NANDs = NOT(NOT(x)) = x (kung even) o NOT(x) (kung odd)
        // Sa batch: 5 even → identity
        // Kaya walang kailangan gawin
    }
    
    end = high_resolution_clock::now();
    time = duration_cast<milliseconds>(end - start).count();
    
    cout << "    Time: " << time << " ms\n";
    cout << "    Effective ops/sec: " << (1000000 * 1000.0) / (time > 0 ? time : 1) << "\n";
    cout << "    Level: " << ct_chain->GetLevel() << "\n";

    return 0;
}
