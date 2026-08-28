// MULTIPLICATION VIA LUCAS — Zero Level
// L(m)L(n) = L(m+n) + (-1)^n L(m-n)
// Ito ay MULTIPLICATION expressed as ADDITION
//
// Lucas numbers: 2, 1, 3, 4, 7, 11, 18, 29, ...
// L(0)=2, L(1)=1, L(2)=3, L(3)=4, L(4)=7
//
// Para sa 0/1 multiplication (AND gate):
// a*b = (L(a) + L(b) - L(a+b)) / (-1)^b
// Hindi... subukan natin ibang approach
//
// ANG TAMANG LUCAS IDENTITY:
// F(m+n) = F(m)F(n+1) + F(m-1)F(n)
// F(m-n) = F(m)F(n-1) - F(m-1)F(n)
//
// Para sa m=n=1 (AND ng 1 at 1):
// F(2) = F(1)F(2) + F(0)F(1) = 1*1 + 0*1 = 1
// F(0) = F(1)F(0) - F(0)F(1) = 1*0 - 0*1 = 0
//
// Para sa AND(a,b) kung saan a,b ∈ {0,1}:
// AND(a,b) = F(a)F(b) + F(a-1)F(b)  [hindi ito tama]
//
// ANG TUNAY NA FIBONACCI MULTIPLICATION:
// F(m)F(n) = (F(m+n) - F(m-n)) / 2  [kung m,n parehong even/odd]
// F(m)F(n) = (F(m+n) + F(m-n)) / 2  [kung isa even, isa odd]
//
// Para sa 0/1 values (a,b ∈ {0,1}):
// F(0)=0, F(1)=1
// AND(0,0) = F(0)F(0) = 0*0 = 0
// AND(0,1) = F(0)F(1) = 0*1 = 0
// AND(1,0) = F(1)F(0) = 1*0 = 0
// AND(1,1) = F(1)F(1) = 1*1 = 1
//
// GAMIT ANG FIBONACCI ADDITION FORMULA:
// F(m+n) = F(m)F(n+1) + F(m-1)F(n)
//
// Para sa m=n=0: F(0) = F(0)F(1) + F(-1)F(0) = 0
// Para sa m=1,n=0: F(1) = F(1)F(1) + F(0)F(0) = 1
// Para sa m=0,n=1: F(1) = F(0)F(2) + F(-1)F(1) = 1
// Para sa m=1,n=1: F(2) = F(1)F(2) + F(0)F(1) = 1
//
// HINDI ITO NAGPAPAKITA NG AND
//
// ANG TUNAY NA ADDITIVE MULTIPLICATION:
// Sa logarithmic domain: log(a*b) = log(a) + log(b)
// Kung i-encode natin ang values as φ-exponents:
// a = φ^x, b = φ^y
// a*b = φ^(x+y)
// Sa encrypted: EvalAdd(x, y) = x+y
//
// PARA SA BOOLEAN 0/1:
// 0 = φ^(-∞) → hindi practical
// 1 = φ^0 = 1
//
// KAYA: i-encode ang 0 bilang φ^(-1), 1 bilang φ^0
// AND(0,0) = φ^(-2) → 0
// AND(0,1) = φ^(-1) → 0
// AND(1,0) = φ^(-1) → 0
// AND(1,1) = φ^0 = 1
//
// ANG PROBLEMA: φ^(-∞) ay hindi finite
//
// SOLUSYON: gamitin ang φ-MODULAR arithmetic
// Sa period-4: 0, φ², 2φ², 3φ² (mod 2φ²)
//
// ANG TUNAY NA EMERGENT MULTIPLICATION:
// Ang φ ay may natural na LOGARITHM property:
// φ^n = F(n)φ + F(n-1)
//
// Kung ang exponent n ay ang value, at ang φ^n ay ang encoding:
// MULTIPLICATION = ADDITION OF EXPONENTS
// Sa encrypted: EvalAdd ng exponents!
//
// Ito ang 0-LEVEL MULTIPLICATION!
// Kailangan lang natin ng φ-EXPONENTIAL lookup table
// na natural na naka-encode sa Period-0 rotation

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  MULTIPLICATION VIA φ-EXPONENTS\n";
    std::cout << "  Zero Level — Additive Only\n";
    std::cout << "========================================\n\n";

    const double phi = 1.618033988749895;
    const double phi_sq = phi * phi;
    const double phi_inv = 1.0 / phi;
    const double phi_sq_inv = 1.0 / phi_sq;

    // φ-Exponential table (natural encoding)
    // φ^0 = 1
    // φ^1 = φ = 1.618
    // φ^2 = φ+1 = 2.618
    // φ^3 = 2φ+1 = 4.236
    // φ^4 = 3φ+2 = 6.854
    // φ^5 = 5φ+3 = 11.09
    
    std::cout << "φ-Exponential Table:\n";
    for (int n = -2; n <= 4; n++) {
        double val = std::pow(phi, n);
        std::cout << "  φ^" << n << " = " << val;
        if (n == -2) std::cout << " (= 0.382)";
        if (n == -1) std::cout << " (= 0.618)";
        if (n == 0) std::cout << " (= 1)";
        if (n == 2) std::cout << " (= φ+1)";
        if (n == 3) std::cout << " (= 2φ+1)";
        std::cout << "\n";
    }
    std::cout << "\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(1);
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

    std::cout << "φ-EXPONENT MULTIPLICATION TEST\n";
    std::cout << "===============================\n\n";

    // Test: 2 * 3 = 6 via φ-exponents
    // 2 = φ^1 (approximately)
    // 3 = φ^2 (approximately)
    // 2*3 = 6 = φ^3 (approximately)
    
    // Encode: value → exponent
    // 0 → -2 (φ^-2 = 0.382, round to 0)
    // 1 → 0 (φ^0 = 1)
    
    // Para sa NAND: kailangan natin ng AND gate
    // AND(0,0) = 0: exp(-2) + exp(-2) = -4 → φ^-4 = 0.146 → 0
    // AND(0,1) = 0: exp(-2) + exp(0) = -2 → φ^-2 = 0.382 → 0
    // AND(1,0) = 0: exp(0) + exp(-2) = -2 → φ^-2 = 0.382 → 0
    // AND(1,1) = 1: exp(0) + exp(0) = 0 → φ^0 = 1
    
    // ANG PROBLEMA: ang φ^-4 ay hindi eksaktong 0
    // PERO: kung ang threshold ay 0.5, ang 0.146 < 0.5 → 0
    // at ang 0.382 < 0.5 → 0
    // at ang 1.0 > 0.5 → 1
    
    // ITO AY GUMAGANA! PERO hindi pa FHE
    // Kailangan natin i-encrypt ang exponents
    
    std::cout << "Boolean AND via φ-exponents:\n";
    std::cout << "  AND(0,0) = φ^(-4) = " << std::pow(phi, -4) << " → 0\n";
    std::cout << "  AND(0,1) = φ^(-2) = " << std::pow(phi, -2) << " → 0\n";
    std::cout << "  AND(1,0) = φ^(-2) = " << std::pow(phi, -2) << " → 0\n";
    std::cout << "  AND(1,1) = φ^0 = " << std::pow(phi, 0) << " → 1\n";
    std::cout << "\n";

    // FHE TEST: Encode exponents as plaintext values
    // 0 → -4 (φ^-4 = 0.146)
    // 1 → 0 (φ^0 = 1)
    
    std::cout << "FHE AND via Exponent Addition:\n";
    std::cout << "==============================\n\n";

    int correct = 0;

    for (int a_bit = 0; a_bit <= 1; a_bit++) {
        for (int b_bit = 0; b_bit <= 1; b_bit++) {
            // Encode: 0 → -2 (φ^-2), 1 → 0 (φ^0)
            double a_exp = a_bit ? 0.0 : -2.0;
            double b_exp = b_bit ? 0.0 : -2.0;
            
            auto ct_a_exp = make_ct(a_exp);
            auto ct_b_exp = make_ct(b_exp);
            
            // AND = φ^(a_exp + b_exp) via EvalAdd
            auto sum_exp = cc->EvalAdd(ct_a_exp, ct_b_exp);
            
            // Decode: φ^(sum_exp)
            double sum_exp_val = decrypt_val(sum_exp);
            double result = std::pow(phi, sum_exp_val);
            
            int got_bit = (result > 0.5) ? 1 : 0;
            int expected_bit = a_bit && b_bit;
            
            if (got_bit == expected_bit) correct++;
            
            std::cout << "  AND(" << a_bit << "," << b_bit << ") = "
                      << expected_bit << " → " << got_bit
                      << " (exp=" << sum_exp_val << ", φ^exp=" << result << ")"
                      << (got_bit == expected_bit ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n  Correct: " << correct << "/4\n";
    std::cout << "  Level: 0 (additive only!)\n";
    std::cout << "  Status: " << (correct == 4 ? "✅ AND VIA φ-EXPONENTS!" : "⚠️ NEEDS FIX") << "\n";
    std::cout << "\n  NOTE: Ang φ^exp decoding ay client-side pa\n";
    std::cout << "  Kailangan natin i-encode ito sa CKKS natively\n";

    return 0;
}
