// FIBONACCI RECURRENCE — 0-LEVEL PERIOD-0 RESEARCH
// x_{n+1} = x_{n-1} - x_n (Fibonacci-like)
// Baka ito ang natural na Period-0 sa FHE!

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  FIBONACCI RECURRENCE RESEARCH\n";
    std::cout << "  x_{n+1} = x_{n-1} - x_n\n";
    std::cout << "  Natural Period-0 Attempt\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(5);
    params.SetScalingModSize(50);
    params.SetBatchSize(256);
    params.SetFirstModSize(60);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    auto slots = cc->GetEncodingParams()->GetBatchSize();

    auto make_ct = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    // ============================================
    // FIBONACCI RECURRENCE: x_{n+1} = x_{n-1} - x_n
    // ============================================
    std::cout << "FIBONACCI RECURRENCE TEST:\n";
    std::cout << "==========================\n\n";
    
    auto x_prev = make_ct(0.0);  // x_0 = 0
    auto x_curr = make_ct(phi_sq);  // x_1 = φ²
    
    std::cout << "  x_0 = " << decrypt_val(x_prev) << "\n";
    std::cout << "  x_1 = " << decrypt_val(x_curr) << "\n\n";
    
    int errors = 0;
    std::vector<double> traj;
    traj.push_back(decrypt_val(x_prev));
    traj.push_back(decrypt_val(x_curr));
    
    for (int i = 2; i < 50; i++) {
        // x_{n+1} = x_{n-1} - x_n (0-level!)
        auto x_next = cc->EvalSub(x_prev, x_curr);
        
        x_prev = x_curr;
        x_curr = x_next;
        
        double v = decrypt_val(x_curr);
        traj.push_back(v);
        
        if (i < 20) {
            std::cout << "  x_" << i << " = " << v 
                      << " level=" << x_curr->GetLevel() << "\n";
        }
    }
    
    // Check for periodicity
    bool has_period = false;
    for (int p = 1; p < 25; p++) {
        bool is_p = true;
        for (int i = 0; i < 20; i++) {
            if (std::abs(traj[i] - traj[i + p]) > 0.001) {
                is_p = false;
                break;
            }
        }
        if (is_p) {
            has_period = true;
            std::cout << "\n  PERIOD FOUND: " << p << " steps\n";
            break;
        }
    }
    
    if (!has_period) {
        std::cout << "\n  WALANG PERIOD — IRRATIONAL!\n";
    }
    
    std::cout << "  Level: " << x_curr->GetLevel() << "\n\n";
    
    // ============================================
    // ALTERNATIVE: x_{n+1} = φ - x_n (period-2)
    //              x_{n+1} = φ² - x_n (period-2)
    //              x_{n+1} = φ - x_n + x_{n-1} (Fibonacci!)
    // ============================================
    std::cout << "ALTERNATIVE FIBONACCI RECURRENCES:\n";
    std::cout << "=================================\n\n";
    
    // Recurrence 1: x_{n+1} = φ - x_n + x_{n-1}
    std::cout << "1. x_{n+1} = φ - x_n + x_{n-1}:\n   ";
    auto a_prev = make_ct(0.0);
    auto a_curr = make_ct(1.0);
    auto ct_phi = make_ct(phi);
    
    std::cout << decrypt_val(a_prev) << " " << decrypt_val(a_curr) << " ";
    
    for (int i = 2; i < 15; i++) {
        auto temp = cc->EvalSub(ct_phi, a_curr);
        auto a_next = cc->EvalAdd(temp, a_prev);
        a_prev = a_curr;
        a_curr = a_next;
        std::cout << decrypt_val(a_curr) << " ";
    }
    std::cout << "\n\n";
    
    // Recurrence 2: x_{n+1} = 2x_n - x_{n-1} (arithmetic)
    std::cout << "2. x_{n+1} = 2x_n - x_{n-1} (arithmetic):\n   ";
    auto b_prev = make_ct(0.0);
    auto b_curr = make_ct(1.0);
    
    std::cout << decrypt_val(b_prev) << " " << decrypt_val(b_curr) << " ";
    
    for (int i = 2; i < 15; i++) {
        auto double_curr = cc->EvalAdd(b_curr, b_curr);
        auto b_next = cc->EvalSub(double_curr, b_prev);
        b_prev = b_curr;
        b_curr = b_next;
        std::cout << decrypt_val(b_curr) << " ";
    }
    std::cout << "\n\n";
    
    // Recurrence 3: x_{n+1} = x_n - x_{n-1} + φ (φ-shifted)
    std::cout << "3. x_{n+1} = x_n - x_{n-1} + φ:\n   ";
    auto c_prev = make_ct(0.0);
    auto c_curr = make_ct(phi_sq);
    
    std::cout << decrypt_val(c_prev) << " " << decrypt_val(c_curr) << " ";
    
    for (int i = 2; i < 15; i++) {
        auto diff = cc->EvalSub(c_curr, c_prev);
        auto c_next = cc->EvalAdd(diff, ct_phi);
        c_prev = c_curr;
        c_curr = c_next;
        std::cout << decrypt_val(c_curr) << " ";
    }
    std::cout << "\n\n";
    
    std::cout << "========================================\n";
    std::cout << "  KEY: Hanapin ang recurrence na:\n";
    std::cout << "  - 0-level\n";
    std::cout << "  - Bounded\n";
    std::cout << "  - Walang period (irrational)\n";
    std::cout << "========================================\n";

    return 0;
}
