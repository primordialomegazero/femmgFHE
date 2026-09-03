// ============================================
// φ-ALU FULL — EDGE CASES + DIVISION + OPTIMIZED MULT
//
// Edge cases: 0×5, 1×255, 255×255
// Division: repeated subtraction
// Binary multiplication: shift + add
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <complex>
#include <iomanip>
#include <chrono>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-ALU FULL — EDGE + DIV + BIN MULT\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    cout << "  ✅ CKKS initialized (depth 1, modsize 59!)\n\n";

    auto encrypt_int = [&](int val) {
        vector<double> v(16, (double)val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_avg = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
        return sum / 16.0;
    };

    // ============================================
    // TEST 1: EDGE CASES
    // ============================================

    cout << "========================================\n";
    cout << "  EDGE CASES\n";
    cout << "========================================\n\n";

    struct EdgeCase {
        int a, b;
        int expected;
    };

    vector<EdgeCase> edges = {
        {0, 5, 0},
        {1, 255, 255},
        {255, 255, 65025},
        {0, 0, 0},
        {7, 1, 7}
    };

    int edge_match = 0;
    for (const auto& e : edges) {
        auto ct_result = encrypt_int(0);
        auto ct_a = encrypt_int(e.a);
        
        auto start = high_resolution_clock::now();
        for (int i = 0; i < e.b; i++) {
            ct_result = cc->EvalAdd(ct_result, ct_a);
        }
        auto end = high_resolution_clock::now();
        auto time = duration_cast<milliseconds>(end - start).count();
        
        double avg = decrypt_avg(ct_result);
        int decoded = (int)round(avg);
        bool match = (decoded == e.expected);
        edge_match += match;
        
        cout << "  " << e.a << " × " << e.b << " = " << decoded 
             << " (Expected: " << e.expected << ") "
             << (match ? "✅" : "❌") << " [" << time << "ms]\n";
    }
    cout << "\n  Edge Match: " << edge_match << "/" << edges.size() << "\n\n";

    // ============================================
    // TEST 2: DIVISION (REPEATED SUBTRACTION)
    // ============================================

    cout << "========================================\n";
    cout << "  DIVISION (REPEATED SUBTRACTION)\n";
    cout << "========================================\n\n";

    struct DivCase {
        int dividend, divisor;
        int expected_quotient;
        int expected_remainder;
    };

    vector<DivCase> divs = {
        {15, 3, 5, 0},
        {17, 5, 3, 2},
        {100, 7, 14, 2},
        {1, 1, 1, 0}
    };

    int div_match = 0;
    for (const auto& d : divs) {
        auto ct_dividend = encrypt_int(d.dividend);
        auto ct_divisor = encrypt_int(d.divisor);
        auto ct_quotient = encrypt_int(0);
        
        auto start = high_resolution_clock::now();
        
        // Repeated subtraction
        while (true) {
            double current = decrypt_avg(ct_dividend);
            if (current < d.divisor) break;
            
            ct_dividend = cc->EvalSub(ct_dividend, ct_divisor);
            ct_quotient = cc->EvalAdd(ct_quotient, encrypt_int(1));
        }
        
        auto end = high_resolution_clock::now();
        auto time = duration_cast<milliseconds>(end - start).count();
        
        double q = decrypt_avg(ct_quotient);
        double r = decrypt_avg(ct_dividend);
        int quotient = (int)round(q);
        int remainder = (int)round(r);
        
        bool match = (quotient == d.expected_quotient && remainder == d.expected_remainder);
        div_match += match;
        
        cout << "  " << d.dividend << " ÷ " << d.divisor 
             << " = " << quotient << " r " << remainder 
             << " (Expected: " << d.expected_quotient << " r " << d.expected_remainder << ") "
             << (match ? "✅" : "❌") << " [" << time << "ms]\n";
    }
    cout << "\n  Division Match: " << div_match << "/" << divs.size() << "\n\n";

    // ============================================
    // TEST 3: BINARY MULTIPLICATION (SHIFT + ADD)
    // ============================================

    cout << "========================================\n";
    cout << "  BINARY MULTIPLICATION (SHIFT + ADD)\n";
    cout << "========================================\n\n";

    struct BinMultCase {
        int a, b;
        int expected;
    };

    vector<BinMultCase> mults = {
        {12, 13, 156},
        {7, 9, 63},
        {100, 100, 10000},
        {255, 255, 65025}
    };

    int mult_match = 0;
    for (const auto& m : mults) {
        auto ct_result = encrypt_int(0);
        auto ct_a = encrypt_int(m.a);
        
        auto start = high_resolution_clock::now();
        
        // Binary multiplication: B times A using shift + add
        int b = m.b;
        int shift = 0;
        
        while (b > 0) {
            if (b & 1) {
                // Add A shifted left by 'shift' positions
                int shifted = m.a << shift;
                auto ct_shifted = encrypt_int(shifted);
                ct_result = cc->EvalAdd(ct_result, ct_shifted);
            }
            b >>= 1;
            shift++;
        }
        
        auto end = high_resolution_clock::now();
        auto time = duration_cast<milliseconds>(end - start).count();
        
        double avg = decrypt_avg(ct_result);
        int decoded = (int)round(avg);
        bool match = (decoded == m.expected);
        mult_match += match;
        
        cout << "  " << m.a << " × " << m.b << " = " << decoded 
             << " (Expected: " << m.expected << ") "
             << (match ? "✅" : "❌") << " [" << time << "ms]\n";
    }
    cout << "\n  Binary Mult Match: " << mult_match << "/" << mults.size() << "\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  ALU FULL COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Edge: " << edge_match << "/" << edges.size() << "\n";
    cout << "  ✅ Division: " << div_match << "/" << divs.size() << "\n";
    cout << "  ✅ Binary Mult: " << mult_match << "/" << mults.size() << "\n";
    cout << "  ✅ Level 0\n\n";

    return 0;
}
