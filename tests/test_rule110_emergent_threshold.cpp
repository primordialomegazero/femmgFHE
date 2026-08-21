// RULE 110 — EMERGENT THRESHOLD SEARCH
// Hanapin ang natural threshold sa φ-domain
// Na nagbibigay ng exact Rule 110 transition

#include <iostream>
#include <vector>
#include <cmath>

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 — EMERGENT THRESHOLD\n";
    std::cout << "  Natural φ-Domain Threshold Search\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;
    const double three_phi_sq = 3 * phi_sq;

    // Rule 110 truth table (binary)
    // 111→0, 110→1, 101→1, 100→0, 011→1, 010→1, 001→1, 000→0
    
    std::cout << "RULE 110 TRUTH TABLE (binary):\n";
    std::cout << "  L C R | Next\n";
    std::cout << "  -------|-----\n";
    std::cout << "  0 0 0 | 0\n";
    std::cout << "  0 0 1 | 1\n";
    std::cout << "  0 1 0 | 1\n";
    std::cout << "  0 1 1 | 1\n";
    std::cout << "  1 0 0 | 0\n";
    std::cout << "  1 0 1 | 1\n";
    std::cout << "  1 1 0 | 1\n";
    std::cout << "  1 1 1 | 0\n\n";
    
    // ============================================
    // SEARCH: Iba't ibang φ-based thresholds
    // ============================================
    std::cout << "THRESHOLD SEARCH:\n";
    std::cout << "=================\n\n";
    
    auto test_threshold = [&](const char* name, auto threshold_fn) {
        int correct = 0;
        std::cout << name << ":\n";
        
        for (int l = 0; l <= 1; l++) {
            for (int c = 0; c <= 1; c++) {
                for (int r = 0; r <= 1; r++) {
                    // Expected
                    int expected;
                    if (l == 1 && c == 1 && r == 1) expected = 0;
                    else if (l == 0 && c == 0 && r == 0) expected = 0;
                    else expected = 1;
                    
                    // φ-domain values
                    double L = l ? phi_sq : 0.0;
                    double C = c ? phi_sq : 0.0;
                    double R = r ? phi_sq : 0.0;
                    
                    double result = threshold_fn(L, C, R);
                    
                    // Decode: > 0.5φ² = 1, else 0
                    int got = (result > phi_sq / 2) ? 1 : 0;
                    
                    if (expected == got) correct++;
                }
            }
        }
        
        std::cout << "  Correct: " << correct << "/8\n\n";
        return correct;
    };
    
    // Threshold 1: 2φ² - (L+C+R)
    test_threshold("1. 2φ² - sum", [&](double L, double C, double R) {
        return two_phi_sq - (L + C + R);
    });
    
    // Threshold 2: 3φ² - (L+C+R)
    test_threshold("2. 3φ² - sum", [&](double L, double C, double R) {
        return three_phi_sq - (L + C + R);
    });
    
    // Threshold 3: |φ² - sum|
    test_threshold("3. |φ² - sum|", [&](double L, double C, double R) {
        double sum = L + C + R;
        return std::abs(phi_sq - sum);
    });
    
    // Threshold 4: φ² - |sum - φ²|
    test_threshold("4. φ² - |sum - φ²|", [&](double L, double C, double R) {
        double sum = L + C + R;
        return phi_sq - std::abs(sum - phi_sq);
    });
    
    // Threshold 5: (L XOR C XOR R) via φ
    test_threshold("5. XOR-like (L + C + R mod 2φ²)", [&](double L, double C, double R) {
        double sum = L + C + R;
        // Modulo 2φ²
        double mod = std::fmod(sum, two_phi_sq);
        return phi_sq - mod;
    });
    
    // Threshold 6: φ²·cos(π·sum/φ²)
    test_threshold("6. φ²·cos(π·sum/φ²)", [&](double L, double C, double R) {
        double sum = L + C + R;
        return phi_sq * std::cos(3.14159 * sum / phi_sq);
    });
    
    // ============================================
    // KEY: Hanapin ang threshold na 8/8!
    // ============================================
    std::cout << "========================================\n";
    std::cout << "  KEY: Ang emergent threshold ay dapat\n";
    std::cout << "  magbigay ng 8/8 correct!\n";
    std::cout << "  Kailangan ng φ-native formula\n";
    std::cout << "  na natural sa φ-domain\n";
    std::cout << "========================================\n";
    
    // ============================================
    // RADICAL: Polynomial sa sum
    // ============================================
    std::cout << "\nPOLYNOMIAL THRESHOLD SEARCH:\n";
    std::cout << "============================\n\n";
    
    // Subukan: f(sum) = a·sum³ + b·sum² + c·sum + d
    // Na nagbibigay ng exact Rule 110
    
    // Sa binary: sum = 0, 1, 2, 3
    // Expected: next = 0, 1, 1, 0 (para sa sum ng 3 bits)
    // Wait, mali — Rule 110 ay hindi lang sum!
    // Ang transition ay: 111→0, 110→1, 101→1, 100→0, 011→1, 010→1, 001→1, 000→0
    
    std::cout << "OBSERVATION:\n";
    std::cout << "============\n";
    std::cout << "  Rule 110 transition (by sum):\n";
    std::cout << "  sum=0 (000) → 0\n";
    std::cout << "  sum=1 (001,010,100) → 1,1,0 (mixed!)\n";
    std::cout << "  sum=2 (011,101,110) → 1,1,1 (lahat 1!)\n";
    std::cout << "  sum=3 (111) → 0\n\n";
    std::cout << "  KEY: Ang sum=1 ay may mixed output (1,1,0)\n";
    std::cout << "  At sum=2 ay lahat 1\n";
    std::cout << "  → Kailangan ng MAS COMPLEX na threshold\n";
    std::cout << "  kaysa sa simple sum!\n\n";
    
    std::cout << "  ANG TAMANG TRANSITION:\n";
    std::cout << "  next = 1 KUNG (L,C,R) ∈ {001,010,011,101,110}\n";
    std::cout << "  next = 0 KUNG (L,C,R) ∈ {000,100,111}\n\n";
    std::cout << "  Mapapansin: next = 1 KUNG may 1 o 2 ones\n";
    std::cout << "  next = 0 KUNG 0 o 3 ones\n";
    std::cout << "  EXCEPT: (100) ay sum=1 pero next=0!\n\n";
    std::cout << "  → Kailangan ng asymmetric threshold!\n";
    
    return 0;
}
