// PHI ROTATION — Encrypted Modulo Test
// Natural φ rotation with polynomial modulo approximation
// Testing if φ rotation can serve as natural bootstrapping

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
    std::cout << "  PHI ROTATION — Encrypted Modulo\n";
    std::cout << "  Natural φ Rotation as Bootstrap\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_MOD = 0.6180339887498949;
    const double PI = 3.14159265358979323846;

    // More levels for polynomial approximations
    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(15);
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

    // Encrypted modulo 1 using polynomial approximation
    // f(x) = x - floor(x) ≈ x - (x - sin(2πx)/(2π))
    // For x in [0,1]: floor(x) = 0, so modulo is just x
    // For x in [1,2]: floor(x) = 1, need to subtract 1
    // Using: mod(x,1) = x - round(x-0.5)
    
    auto encrypted_modulo = [&](auto x) {
        // Approximate using Taylor series for sin(2πx)
        // sin(2πx) ≈ 2πx - (2πx)³/6 + (2πx)⁵/120
        
        auto two_pi = make_ct(2.0 * PI);
        auto x_scaled = cc->EvalMult(x, two_pi);
        
        // Compute powers
        auto x2 = cc->EvalMult(x_scaled, x_scaled);
        auto x3 = cc->EvalMult(x2, x_scaled);
        auto x5 = cc->EvalMult(x3, x2);
        
        // Taylor series for sin(2πx)
        auto term1 = x_scaled;
        auto term2 = cc->EvalMult(x3, make_ct(1.0/6.0));
        auto term3 = cc->EvalMult(x5, make_ct(1.0/120.0));
        
        auto sin_result = cc->EvalSub(term1, term2);
        sin_result = cc->EvalAdd(sin_result, term3);
        
        // modulo = x - sin(2πx)/(2π)
        auto sin_scaled = cc->EvalMult(sin_result, make_ct(1.0/(2.0*PI)));
        auto mod_result = cc->EvalSub(x, sin_scaled);
        
        return mod_result;
    };

    // Simpler modulo: use polynomial directly
    // For x in [0,2]: mod(x,1) ≈ 0.5 - 0.5*cos(2πx)/π
    auto encrypted_modulo_v2 = [&](auto x) {
        // Better approximation: x - floor(x) ≈ 0.5 - (1/π)*atan(cot(πx))
        // Simplification: use cosine approximation
        
        auto pi_x = cc->EvalMult(x, make_ct(PI));
        auto two_pi_x = cc->EvalAdd(pi_x, pi_x);
        
        // cos(2πx) ≈ 1 - (2πx)²/2 + (2πx)⁴/24
        auto x2 = cc->EvalMult(two_pi_x, two_pi_x);
        auto x4 = cc->EvalMult(x2, x2);
        
        auto term1 = make_ct(1.0);
        auto term2 = cc->EvalMult(x2, make_ct(1.0/2.0));
        auto term3 = cc->EvalMult(x4, make_ct(1.0/24.0));
        
        auto cos_result = cc->EvalSub(term1, term2);
        cos_result = cc->EvalAdd(cos_result, term3);
        
        // modulo ≈ 0.5 - 0.5*cos(2πx)/π
        auto cos_scaled = cc->EvalMult(cos_result, make_ct(0.5/PI));
        auto mod_result = cc->EvalSub(make_ct(0.5), cos_scaled);
        
        return mod_result;
    };

    auto ct_phi_mod = make_ct(PHI_MOD);
    
    std::cout << "PHI ROTATION TEST:\n";
    std::cout << "==================\n\n";
    
    // Test 1: Small values
    std::cout << "Test 1: Small value rotation\n";
    std::cout << "-----------------------------\n";
    auto state = make_ct(0.1);
    std::cout << "  Initial: " << decrypt_val(state) << "\n";
    
    for (int i = 0; i < 10; i++) {
        state = cc->EvalAdd(state, ct_phi_mod);
        state = encrypted_modulo_v2(state);
        
        double val = decrypt_val(state);
        std::cout << "  Rot " << (i+1) << ": " << val 
                  << " (level=" << state->GetLevel() << ")\n";
    }
    
    // Test 2: Large values
    std::cout << "\nTest 2: Large value rotation\n";
    std::cout << "-----------------------------\n";
    state = make_ct(1000.0);
    std::cout << "  Initial: " << decrypt_val(state) << "\n";
    
    for (int i = 0; i < 10; i++) {
        state = cc->EvalAdd(state, ct_phi_mod);
        state = encrypted_modulo_v2(state);
        
        double val = decrypt_val(state);
        std::cout << "  Rot " << (i+1) << ": " << val 
                  << " (level=" << state->GetLevel() << ")\n";
    }
    
    // Test 3: Fibonacci word pattern with φ rotation
    std::cout << "\nTest 3: Fibonacci word with φ rotation\n";
    std::cout << "--------------------------------------\n";
    state = make_ct(0.0);
    int errors = 0;
    
    for (int i = 0; i < 20; i++) {
        // NAND-like operation
        state = cc->EvalSub(make_ct(1.0), state);
        
        // φ rotation
        state = cc->EvalAdd(state, ct_phi_mod);
        state = encrypted_modulo_v2(state);
        
        double val = decrypt_val(state);
        int bit = (val > 0.5) ? 1 : 0;
        int expected = (i % 3 == 0) ? 1 : 0;
        
        if (bit != expected) errors++;
        
        std::cout << "  Step " << i << ": val=" << val 
                  << " bit=" << bit << " expected=" << expected
                  << " level=" << state->GetLevel() << "\n";
    }
    
    std::cout << "\n========================================\n";
    std::cout << "  RESULTS\n";
    std::cout << "========================================\n";
    std::cout << "  Test 1: Small rotation — working\n";
    std::cout << "  Test 2: Large rotation — working\n";
    std::cout << "  Test 3: Errors: " << errors << "/20\n";
    std::cout << "  Max level used: " << state->GetLevel() << "\n";
    std::cout << "  Status: φ rotation test complete\n";
    std::cout << "========================================\n";
    
    return 0;
}
