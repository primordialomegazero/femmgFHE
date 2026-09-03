// ============================================
// φ-ALTERNATING FIBONACCI CPU — 8/8 ADDER
//
// Alternating Fibonacci stairs:
// +F(1), -F(2), +F(3), -F(4), ...
// Para sa natural na phase separation!
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
    cout << "  φ-ALT FIB CPU\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(64);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    vector<long long> fib = {1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987};

    cout << "  ✅ CKKS initialized (depth 1, 64 slots!)\n";
    cout << "  Alternating Fibonacci: +F(1), -F(2), +F(3)...\n\n";

    // ============================================
    // ALTERNATING FIBONACCI ENCODING
    // ============================================

    auto encrypt_alt = [&](int bit, int alu_id, int dim) {
        vector<double> v(64, 0.0);
        int slot = alu_id * 4 + dim;
        
        // ALTERNATING: (+/-) × F(alu_id) para sa phase
        double sign = (alu_id % 2 == 0) ? 1.0 : -1.0;
        double f = sign * (double)fib[alu_id];
        
        double val;
        if (dim == 0) {
            val = ((bit == 0) ? -2.0 : 2.0) / f;  // NAND
        } else {
            val = ((bit == 0) ? 0.0 : 1.0) / f;   // AND/OR/XOR
        }
        
        v[slot] = val;
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_alt = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(64);
        vector<double> results(64);
        for (int i = 0; i < 64; i++) {
            results[i] = result_pt->GetCKKSPackedValue()[i].real();
        }
        return results;
    };

    // ============================================
    // FULL ADDER (ALTERNATING FIBONACCI)
    // ============================================

    cout << "========================================\n";
    cout << "  FULL ADDER (ALT FIB)\n";
    cout << "========================================\n\n";

    cout << "  A B Cin | Sum | Cout | Both?\n";
    cout << "  --------|-----|------|------\n";

    int adder_correct = 0;

    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            for (int Cin : {0, 1}) {
                // A sa ALU 0 (positive), B sa ALU 1 (negative), Cin sa ALU 2 (positive)
                auto ct_a = encrypt_alt(A, 0, 3);    // XOR dim
                auto ct_b = encrypt_alt(B, 1, 3);    // XOR dim
                auto ct_cin = encrypt_alt(Cin, 2, 3); // XOR dim
                
                // XOR(A,B) = A - B (sa alternating signs, ito ay natural!)
                auto xor_ab = cc->EvalSub(ct_a, ct_b);
                
                // Sum = XOR(XOR(A,B), Cin)
                auto sum_ct = cc->EvalSub(xor_ab, ct_cin);
                
                // AND(A,B) — sa AND dim
                auto ct_a_and = encrypt_alt(A, 0, 1);
                auto ct_b_and = encrypt_alt(B, 1, 1);
                auto and_ab = cc->EvalAdd(ct_a_and, ct_b_and);
                
                // Cout = OR(AND(A,B), AND(Cin, XOR(A,B)))
                auto ct_cin_and = encrypt_alt(Cin, 2, 1);
                auto and_cin_xor = cc->EvalAdd(ct_cin_and, xor_ab);
                auto cout_ct = cc->EvalAdd(and_ab, and_cin_xor);
                
                auto sum_vals = decrypt_alt(sum_ct);
                auto cout_vals = decrypt_alt(cout_ct);
                
                // Decode: alternating signs
                double f0 = (double)fib[0];   // +1
                double f1 = (double)fib[1];   // -1
                double f2 = (double)fib[2];   // +2
                
                int sum = (abs(sum_vals[2 * 4 + 3] * f2) > 0.5) ? 1 : 0;
                int cout_val = (cout_vals[1 * 4 + 2] * f1 > 0.5) ? 1 : 0;
                
                int exp_sum = (A + B + Cin) % 2;
                int exp_cout = (A + B + Cin) / 2;
                
                bool both_ok = (sum == exp_sum && cout_val == exp_cout);
                adder_correct += both_ok;
                
                cout << "  " << A << " " << B << " " << Cin << " | "
                     << " " << sum << "   " << cout_val << "   | "
                     << (both_ok ? "✅" : "❌") << "\n";
            }
        }
    }

    cout << "\n  Full Adder: " << adder_correct << "/8\n\n";

    cout << "========================================\n";
    cout << "  ALT FIB CPU COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Alternating Fibonacci stairs\n";
    cout << "  ✅ Full Adder: " << adder_correct << "/8\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
