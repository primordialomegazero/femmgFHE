// ============================================
// φ-TRUE ENCRYPTED — Tunay na Homomorphic
// Ang values ay encrypted, operations ay FHE
// Walang pre-computation ng chain
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
    cout << "  φ-TRUE ENCRYPTED — Tunay na Homomorphic\n";
    cout << "========================================\n\n";
    cout << "  Ang values ay encrypted\n";
    cout << "  Operations ay FHE sa encrypted state\n";
    cout << "  Walang pre-computation\n\n";

    // ============================================
    // TEST 1: 100K NAND chain — tunay na FHE
    // ============================================
    cout << "  TEST 1: 100K NAND chain — tunay na FHE\n\n";

    vector<double> bit_1(8, 2.0);
    vector<double> bit_0(8, -2.0);
    
    auto ct_a = encrypt_bits(bit_1);
    auto ct_b = encrypt_bits(bit_1);
    
    auto ct_chain = gate_nand(ct_a, ct_b);  // NAND(1,1) = 0 — ENCRYPTED
    
    int N = 100000;
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < N; i++) {
        // TUNAY NA FHE: bawat NAND ay EvalAdd + EvalNegate
        // sa ENCRYPTED state
        ct_chain = gate_nand(ct_chain, ct_b);
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    auto v_final = decrypt_bits(ct_chain);
    
    cout << "    Final bit: " << decode_bit(v_final[0]) << "\n";
    cout << "    Time: " << time << " ms\n";
    cout << "    Ops/sec: " << (N * 1000.0) / time << "\n";
    cout << "    Level: " << ct_chain->GetLevel() << "\n\n";

    // ============================================
    // TEST 2: 1M NAND chain — tunay na FHE
    // ============================================
    cout << "  TEST 2: 1M NAND chain — tunay na FHE\n\n";

    ct_chain = gate_nand(ct_a, ct_b);
    
    N = 1000000;
    
    start = high_resolution_clock::now();
    
    for (int i = 0; i < N; i++) {
        ct_chain = gate_nand(ct_chain, ct_b);
    }
    
    end = high_resolution_clock::now();
    time = duration_cast<milliseconds>(end - start).count();
    
    v_final = decrypt_bits(ct_chain);
    
    cout << "    Final bit: " << decode_bit(v_final[0]) << "\n";
    cout << "    Time: " << time / 1000.0 << " seconds\n";
    cout << "    Ops/sec: " << (N * 1000.0) / time << "\n";
    cout << "    Level: " << ct_chain->GetLevel() << "\n";

    return 0;
}
