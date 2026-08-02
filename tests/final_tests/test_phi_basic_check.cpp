#include <iostream>
#include <cmath>

int main() {
    const double psi = 0.6180339887498949;
    const double psi2 = psi * psi;
    
    // (A×B) + (C×D)
    double v1=0.5, v2=0.3, v3=0.2, v4=0.777;
    
    // A×B: raw_ab = v1*v2 + ψ(v1+v2) - (2ψ-1)? Let's check
    // Pure mul formula: corrected = raw - ψ(v1+v2) + (2ψ-1)
    // So raw = corrected + ψ(v1+v2) - (2ψ-1) = v1*v2 + ψ(v1+v2) - (2ψ-1)
    double raw_ab = v1*v2 + psi*(v1+v2) - (2*psi-1);
    double expected_ab = v1*v2;
    
    // C×D: raw_cd = v3*v4 + ψ(v3+v4) - (2ψ-1)
    double raw_cd = v3*v4 + psi*(v3+v4) - (2*psi-1);
    double expected_cd = v3*v4;
    
    std::cout << "A×B: raw=" << raw_ab << ", expected=" << expected_ab << "\n";
    std::cout << "C×D: raw=" << raw_cd << ", expected=" << expected_cd << "\n\n";
    
    // Now add them: (A×B)+(C×D)
    // After mul, state_ab = (raw_ab+ψ, 1)? No — state is the ratio form
    // ratio_ab = raw_ab + psi
    // ratio_cd = raw_cd + psi
    double ratio_ab = raw_ab + psi;
    double ratio_cd = raw_cd + psi;
    
    // Add: ratio_sum = ratio_ab + ratio_cd (since b components = 1)
    double ratio_sum = ratio_ab + ratio_cd;
    double raw_sum = ratio_sum - psi;
    
    double expected_sum = expected_ab + expected_cd;
    
    std::cout << "After (A×B)+(C×D):\n";
    std::cout << "  ratio_ab=" << ratio_ab << "\n";
    std::cout << "  ratio_cd=" << ratio_cd << "\n";
    std::cout << "  ratio_sum=" << ratio_sum << "\n";
    std::cout << "  raw_sum=" << raw_sum << "\n";
    std::cout << "  expected=" << expected_sum << "\n";
    std::cout << "  needed correction = " << raw_sum - expected_sum << "\n";
    
    // Derive formula:
    // raw_sum = (ratio_ab + ratio_cd) - psi
    //         = (raw_ab+ψ + raw_cd+ψ) - ψ
    //         = raw_ab + raw_cd + ψ
    //         = [v1*v2 + ψ(v1+v2) - (2ψ-1)] + [v3*v4 + ψ(v3+v4) - (2ψ-1)] + ψ
    //         = v1*v2 + v3*v4 + ψ(v1+v2+v3+v4) - 2(2ψ-1) + ψ
    double derived = v1*v2 + v3*v4 + psi*(v1+v2+v3+v4) - 2*(2*psi-1) + psi;
    std::cout << "  derived raw = " << derived << " (match? " << (std::abs(derived-raw_sum)<1e-10 ? "YES" : "NO") << ")\n";
    
    // Correction:
    // corrected = raw_sum - ψ(v1+v2+v3+v4) + 2(2ψ-1) - ψ
    double correction = psi*(v1+v2+v3+v4) - 2*(2*psi-1) + psi;
    double corrected = raw_sum - correction;
    std::cout << "  correction = " << correction << "\n";
    std::cout << "  corrected = " << corrected << "\n";
    std::cout << "  error = " << corrected - expected_sum << "\n\n";
    
    // For M multiplies then 1 add:
    // raw = sum of (vi*vj) + ψ×sum of all v's - M×(2ψ-1) + ψ
    // corrected = raw - ψ×sum_v_all + M×(2ψ-1) - ψ
    
    std::cout << "=== GENERAL: M muls + 1 add ===\n";
    for (int M = 2; M <= 4; M++) {
        // Simulate M multiplies of pairs then add
        double sum_products = 0.0;
        double sum_all_v = 0.0;
        double raw_total = 0.0;
        
        for (int i = 0; i < M; i++) {
            double va = 0.5 + i*0.1;
            double vb = 0.3 + i*0.1;
            sum_products += va * vb;
            sum_all_v += va + vb;
            raw_total += va*vb + psi*(va+vb) - (2*psi-1);
        }
        // Add them all: raw = sum of raws + (M-1)×ψ
        double raw_final = raw_total + (M-1)*psi;
        double correction_M = psi*sum_all_v - M*(2*psi-1) + (M-1)*psi;
        double corrected_M = raw_final - correction_M;
        
        std::cout << "  M=" << M << ": raw=" << raw_final << ", corrected=" << corrected_M;
        std::cout << ", expected=" << sum_products << ", error=" << corrected_M-sum_products << "\n";
    }
    
    return 0;
}
