#include <iostream>
#include <vector>
#include <array>
#include <cmath>

using namespace std;

// Representasyon ng Z[ζ_5] elements
// a + bζ_5 + cζ_5² + dζ_5³
// (degree 4, kasi ζ_5⁴ = -1 - ζ_5 - ζ_5² - ζ_5³)
struct Zeta5Element {
    array<int64_t, 4> coeff; // {a, b, c, d}
    
    Zeta5Element(int64_t a = 0, int64_t b = 0, int64_t c = 0, int64_t d = 0) 
        : coeff({a, b, c, d}) {}
    
    Zeta5Element operator+(const Zeta5Element& other) const {
        Zeta5Element result;
        for (int i = 0; i < 4; i++) {
            result.coeff[i] = coeff[i] + other.coeff[i];
        }
        return result;
    }
    
    Zeta5Element operator-(const Zeta5Element& other) const {
        Zeta5Element result;
        for (int i = 0; i < 4; i++) {
            result.coeff[i] = coeff[i] - other.coeff[i];
        }
        return result;
    }
    
    // Multiplication sa Z[ζ_5] gamit ang Φ_5(x) = x⁴ + x³ + x² + x + 1
    Zeta5Element operator*(const Zeta5Element& other) const {
        array<int64_t, 7> temp = {0, 0, 0, 0, 0, 0, 0};
        
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                temp[i + j] += coeff[i] * other.coeff[j];
            }
        }
        
        // Reduce gamit ang x⁴ = -1 - x - x² - x³
        // x⁵ = -x - x² - x³ - x⁴ = 1
        // x⁶ = x
        
        array<int64_t, 4> result = {0, 0, 0, 0};
        
        // Term na may degree 4: x⁴ = -1 - x - x² - x³
        result[0] -= temp[4];
        result[1] -= temp[4];
        result[2] -= temp[4];
        result[3] -= temp[4];
        
        // Term na may degree 5: x⁵ = 1
        result[0] += temp[5];
        
        // Term na may degree 6: x⁶ = x
        result[1] += temp[6];
        
        // Lower terms
        for (int i = 0; i < 4; i++) {
            result[i] += temp[i];
        }
        
        return Zeta5Element(result[0], result[1], result[2], result[3]);
    }
};

int main() {
    cout << "========================================\n";
    cout << "  NAND SA Z[ζ_5] - IMPLEMENTATION\n";
    cout << "  Golden ratio NAND na may period-2\n";
    cout << "========================================\n\n";

    // φ = 1 + ζ_5 + ζ_5⁴ = 1 + ζ_5 + (-1 - ζ_5 - ζ_5² - ζ_5³)
    //   = -ζ_5² - ζ_5³
    Zeta5Element phi(0, 0, -1, -1);
    Zeta5Element zero(0, 0, 0, 0);
    Zeta5Element one(1, 0, 0, 0);
    
    cout << "1. GOLDEN RATIO SA Z[ζ_5]:\n";
    cout << "   φ = (0, 0, -1, -1)\n";
    cout << "   Ibig sabihin: φ = -ζ_5² - ζ_5³\n\n";

    // NAND_norm(x,y) = (φ - xy + 1) / φ
    // Sa Z[ζ_5], division by φ ay multiply by inv_phi
    // inv_phi = φ - 1 = -1 - ζ_5² - ζ_5³
    
    Zeta5Element inv_phi(-1, 0, -1, -1);
    
    auto nand_norm = [&](Zeta5Element x, Zeta5Element y) -> Zeta5Element {
        auto xy = x * y;
        auto result = phi - xy + one;
        return result * inv_phi;
    };
    
    cout << "2. NAND_norm TRUTH TABLE:\n";
    cout << "-----------------------------------\n";
    
    auto val0 = zero;
    auto val1 = phi;
    
    auto r00 = nand_norm(val0, val0);
    auto r01 = nand_norm(val0, val1);
    auto r10 = nand_norm(val1, val0);
    auto r11 = nand_norm(val1, val1);
    
    cout << "   NAND(0,0) = (" << r00.coeff[0] << "," << r00.coeff[1] 
         << "," << r00.coeff[2] << "," << r00.coeff[3] << ")\n";
    cout << "   NAND(0,1) = (" << r01.coeff[0] << "," << r01.coeff[1] 
         << "," << r01.coeff[2] << "," << r01.coeff[3] << ")\n";
    cout << "   NAND(1,0) = (" << r10.coeff[0] << "," << r10.coeff[1] 
         << "," << r10.coeff[2] << "," << r10.coeff[3] << ")\n";
    cout << "   NAND(1,1) = (" << r11.coeff[0] << "," << r11.coeff[1] 
         << "," << r11.coeff[2] << "," << r11.coeff[3] << ")\n\n";

    cout << "3. PERIOD-2 VERIFICATION:\n";
    cout << "-----------------------------------\n";
    
    // NAND(NAND(0,0), NAND(0,0)) dapat 0
    auto inner0 = nand_norm(val0, val0);
    auto outer0 = nand_norm(inner0, inner0);
    cout << "   NAND²(0) = (" << outer0.coeff[0] << "," << outer0.coeff[1] 
         << "," << outer0.coeff[2] << "," << outer0.coeff[3] << ")\n";
    cout << "   Expected: (0,0,0,0)\n\n";
    
    // NAND(NAND(1,1), NAND(1,1)) dapat 1
    auto inner1 = nand_norm(val1, val1);
    auto outer1 = nand_norm(inner1, inner1);
    cout << "   NAND²(1) = (" << outer1.coeff[0] << "," << outer1.coeff[1] 
         << "," << outer1.coeff[2] << "," << outer1.coeff[3] << ")\n";
    cout << "   Expected: (0,0,-1,-1) = φ\n\n";

    cout << "4. DEEP CHAIN (100 NANDs):\n";
    cout << "-----------------------------------\n";
    
    auto current = zero;
    int errors = 0;
    
    for (int i = 1; i <= 100; i++) {
        current = nand_norm(current, current);
        
        // Dapat period-2: 0 → φ → 0 → φ → ...
        bool is_phi = (current.coeff[0] == 0 && current.coeff[1] == 0 
                       && current.coeff[2] == -1 && current.coeff[3] == -1);
        bool is_zero = (current.coeff[0] == 0 && current.coeff[1] == 0 
                        && current.coeff[2] == 0 && current.coeff[3] == 0);
        
        bool expected_phi = (i % 2 == 1);
        
        if (expected_phi && !is_phi) {
            errors++;
            if (errors <= 3) {
                cout << "   Iter " << i << ": FAIL (expected φ)\n";
            }
        }
        if (!expected_phi && !is_zero) {
            errors++;
            if (errors <= 3) {
                cout << "   Iter " << i << ": FAIL (expected 0)\n";
            }
        }
        
        if (i % 10 == 0) {
            cout << "   " << i << " NANDs: " << (errors == 0 ? "OK" : "FAIL") << "\n";
        }
    }
    
    cout << "\n========================================\n";
    cout << "  RESULT:\n";
    cout << "  - 100 NANDs: " << (errors == 0 ? "ALL CORRECT" : "WITH ERRORS") << "\n";
    cout << "  - Errors: " << errors << "\n";
    cout << "  - Ito ay exact arithmetic sa Z[ζ_5]\n";
    cout << "  - Walang noise, walang corruption\n";
    cout << "========================================\n";

    return 0;
}
