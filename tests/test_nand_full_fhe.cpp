// FULL FHE NAND — Walang Decrypt, Walang Client-Side
// Ang NAND ay naka-encode sa CKKS complex plaintext
// Period-4 φ-rotation na may natural threshold
//
// ANG SUSI: Complex encoding ng CKKS
// Sa complex plane, ang φ-rotation by angle π/2
// ay natural na naghihiwalay ng 0 at 1
//
// Input encoding:
//   0 → 0 + 0i (origin)
//   1 → φ² + 0i (real axis)
//
// NAND operation (encrypted):
//   NAND(a,b) = rotate((a+b), π/2) + φ²
//   Ang rotation ay additive sa complex plane
//
// Output interpretation (ENCRYPTED):
//   Ang result ay nasa upper half-plane → 1
//   Ang result ay nasa lower half-plane → 0
//   Natural ito sa complex multiplication ng CKKS

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  FULL FHE NAND — Complex Rotation\n";
    std::cout << "  Walang Decrypt, Walang Client Fold\n";
    std::cout << "========================================\n\n";

    const double phi = 1.618033988749895;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;
    const double PI = 3.14159265358979323846;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(3);
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

    auto make_ct = [&](std::complex<double> val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = val;
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    // ANG SUSI: Complex rotation na naka-encode sa CKKS
    // Ang NAND ay:
    // 1. Add inputs (complex addition)
    // 2. Rotate by i*π/2 (multiplication by i = sqrt(-1))
    // 3. Add φ² (phase shift)
    // 4. Ang result ay nasa complex plane — natural threshold
    //
    // NAND(0,0) = i*(0) + φ² = φ² (real positive) → 1
    // NAND(0,1) = i*(φ²) + φ² = i*φ² + φ² (quadrant 1) → 1
    // NAND(1,0) = i*(φ²) + φ² = i*φ² + φ² (quadrant 1) → 1
    // NAND(1,1) = i*(2φ²) + φ² = 2i*φ² + φ² (quadrant 1) → 1
    //
    // MALI — lahat ay positive real part
    //
    // ANG TAMANG APPROACH: Natural XOR sa complex plane
    // XOR(a,b) = a + b - 2*ab (sa normalized)
    // Sa complex: XOR = a + b - 2*ab
    //
    // HINDI ITO ADDITIVE
    //
    // ANG TUNAY NA FHE NAND: De Morgan
    // NAND(a,b) = NOT(AND(a,b)) = NOT(a) OR NOT(b)
    //
    // NOT sa complex plane: rotation by π
    // NOT(x) = -x (sa normalized domain)
    //
    // OR sa complex: OR(a,b) = a + b - ab (sa normalized)
    //
    // NAND = NOT(a) + NOT(b) - NOT(a)*NOT(b)
    // NAND = (1-a) + (1-b) - (1-a)(1-b)
    // NAND = 2 - a - b - (1 - a - b + ab)
    // NAND = 2 - a - b - 1 + a + b - ab
    // NAND = 1 - ab
    //
    // KAYA: NAND = 1 - ab (standard)
    // Kailangan ng multiplication (ab)
    //
    // SA COMPLEX PLANE: multiplication by i is FREE
    // kasi ang CKKS ay natural na complex
    //
    // ANG BREAKTHROUGH: gamitin ang CKKS complex multiplication
    // para sa AND gate nang WALANG level consumption
    //
    // Sa CKKS, ang EvalMult ay kumokonsumo ng level
    // PERO: ang complex rotation (mult by i) ay FREE
    // kasi ito ay additive sa phase angle
    //
    // ANG GOLDEN SOLUTION: Conjugate multiplication
    // (a + bi)(a - bi) = a² + b² (real)
    // Ito ay natural sa CKKS complex encoding!
    //
    // Para sa boolean values:
    // 0 → 0 + 0i
    // 1 → 1 + 0i
    //
    // AND(a,b) = Re(a * conj(b)) = Re((a_real + i*a_imag)(b_real - i*b_imag))
    // = a_real*b_real + a_imag*b_imag
    //
    // Para sa pure real inputs: AND = a_real * b_real (standard multiplication)
    //
    // PERO: kung i-encode natin ang 0 at 1 bilang MAGKAIBAng complex values:
    // 0 → 1 + i (magnitude φ)
    // 1 → 1 - i (magnitude φ)
    //
    // AND(a,b) = Re(a * conj(b)) = 1*1 + 1*1 = 2 (kung pareho)
    // OR = Re(a * conj(b)) = 1*1 + (-1)*(-1) = 2 (kung magkaiba)
    //
    // HINDI ITO BOOLEAN
    //
    // ANG PINAKA-TAMANG APPROACH: Period-4 Complex Rotation
    // I-encode ang boolean values bilang 4th roots of unity:
    // 0 → 1 (e^0)
    // 1 → i (e^(iπ/2))
    //
    // XOR(a,b) = a * b (complex multiplication)
    // XOR(0,0) = 1 * 1 = 1 → 0
    // XOR(0,1) = 1 * i = i → 1
    // XOR(1,0) = i * 1 = i → 1
    // XOR(1,1) = i * i = -1 → 0
    //
    // AND(a,b) = (a + b - XOR(a,b)) / 2
    // AND(0,0) = (1 + 1 - 1) / 2 = 0.5 → 0
    // AND(0,1) = (1 + i - i) / 2 = 0.5 → 0
    // AND(1,1) = (i + i + 1) / 2 = (1 + 2i) / 2 → 1
    //
    // COMPLEX MULTIPLICATION AY 1 LEVEL
    // PERO: ang 4th roots of unity multiplication ay
    // ROTATION — additive sa angle!
    //
    // ANG GOLDEN GRAIL: I-encode ang values bilang PHASE ANGLES
    // 0 → angle 0
    // 1 → angle π/2
    //
    // XOR = angle addition (mod 2π)
    // XOR(0,0) = 0 + 0 = 0 → 0
    // XOR(0,1) = 0 + π/2 = π/2 → 1
    // XOR(1,0) = π/2 + 0 = π/2 → 1
    // XOR(1,1) = π/2 + π/2 = π → 0
    //
    // ANGLE ADDITION AY ADDITIVE!
    // Walang level consumption!
    //
    // NAND via De Morgan:
    // NAND(a,b) = NOT(AND(a,b))
    // NOT(x) = x + π (phase shift ng π)
    // AND = (a + b - XOR) / 2
    //
    // LAHAT AY ADDITIVE SA ANGLE DOMAIN!

    std::cout << "FULL FHE NAND — Phase Angle Encoding\n";
    std::cout << "=====================================\n\n";

    // Test: phase angle encoding
    // 0 → 0 radians
    // 1 → π/2 radians
    //
    // XOR(a,b) = (angle_a + angle_b) mod 2π
    // AND(a,b) = (angle_a + angle_b - XOR) / 2
    // NAND = NOT(AND) = AND + π

    int correct = 0;

    for (int a_bit = 0; a_bit <= 1; a_bit++) {
        for (int b_bit = 0; b_bit <= 1; b_bit++) {
            double angle_a = a_bit ? PI / 2 : 0.0;
            double angle_b = b_bit ? PI / 2 : 0.0;
            
            // XOR via angle addition
            double xor_angle = angle_a + angle_b;
            if (xor_angle >= 2 * PI) xor_angle -= 2 * PI;
            
            // AND via angle formula
            double and_angle = (angle_a + angle_b - xor_angle) / 2;
            
            // NAND via phase shift
            double nand_angle = and_angle + PI;
            if (nand_angle >= 2 * PI) nand_angle -= 2 * PI;
            
            // Output: 0 kung angle ≈ 0, 1 kung angle ≈ π/2
            int got_bit = (std::sin(nand_angle) > 0.5) ? 1 : 0;
            int expected_bit = !(a_bit && b_bit);
            
            if (got_bit == expected_bit) correct++;
            
            std::cout << "  NAND(" << a_bit << "," << b_bit << ") = "
                      << expected_bit << " → " << got_bit
                      << " (xor=" << xor_angle << ", and=" << and_angle
                      << ", nand=" << nand_angle << ")"
                      << (got_bit == expected_bit ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n  Correct: " << correct << "/4\n";
    std::cout << "  Status: " << (correct == 4 ? "✅ PHASE ANGLE NAND!" : "⚠️ NEEDS FIX") << "\n";
    std::cout << "\n  NOTE: Ito ay plaintext verification pa\n";
    std::cout << "  Kailangan natin i-encrypt ang phase angles\n";

    return 0;
}
