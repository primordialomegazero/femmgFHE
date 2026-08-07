/**
 * FGG-BASED MODULO REDUCTION — RIGOROUS TEST
 * 
 * Tests FGG_Mod against std::fmod for:
 * - Positive integers
 * - Negative numbers
 * - Edge cases (zero divisor, zero dividend, small fractions)
 * - Large values
 * 
 * φ·ψ = -1 foundation
 * FGG(v, 3) = |v| — canonical absolute value
 * FGG_Mod extends this to canonical residue modulo p
 */

#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <string>

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

double FGG(double v, int depth = 3) {
    double current = v;
    for (int d = 0; d < depth; d++) {
        if (d % 2 == 0)
            current = std::abs(current * PHI * PSI);
        else
            current = std::abs(current * PSI * PHI);
    }
    return current;
}

/**
 * FGG_Mod: Canonical modulo reduction via Fractal Golden Gate.
 * 
 * THEORY:
 *   FGG(v, 3) = |v| collapses any value to its canonical absolute value.
 *   The same mechanism, when applied to x/p, extracts the fractional part
 *   which corresponds to the residue modulo p.
 * 
 *   scaled = x / p          — normalize to unit interval
 *   collapsed = FGG(scaled)  — collapse to |scaled|
 *   fractional = collapsed - floor(collapsed) — extract residue fraction
 *   result = fractional * p  — scale back to original modulus
 * 
 * HANDLING NEGATIVES:
 *   fmod(x, p) preserves the sign of x.
 *   FGG_Mod uses abs-based collapse, so it returns [0, p) always.
 *   To match fmod exactly: if x < 0 and result != 0, return result - p.
 */
double FGG_Mod(double x, double p) {
    if (p == 0.0) return std::nan("");
    if (x == 0.0) return 0.0;
    
    bool negative_input = (x < 0);
    double abs_x = std::abs(x);
    double abs_p = std::abs(p);
    
    double scaled = abs_x / abs_p;
    double collapsed = FGG(scaled, 3);
    double fractional = collapsed - std::floor(collapsed);
    double result = fractional * abs_p;
    
    // Ensure result is in [0, abs_p)
    result = std::fmod(result, abs_p);
    if (result < 0) result += abs_p;
    
    // Match fmod behavior: preserve sign of x
    if (negative_input && result != 0.0) {
        result = result - abs_p;
    }
    
    // If p is negative, fmod preserves sign of p's effect
    // But FGG_Mod uses abs_p, so we're consistent with absolute modulus
    
    return result;
}

int main() {
    std::cout << "\n";
    std::cout << "================================================================================\n";
    std::cout << "  FGG-BASED MODULO — RIGOROUS TEST SUITE\n";
    std::cout << "  Comparison: FGG_Mod vs std::fmod\n";
    std::cout << "================================================================================\n\n";
    
    struct TestCase {
        double x, p;
        std::string description;
    };
    
    std::vector<TestCase> tests = {
        // Positive integers
        {17.0, 5.0, "17 mod 5 (positive ints)"},
        {42.0, 5.0, "42 mod 5"},
        {100.0, 7.0, "100 mod 7"},
        {256.0, 16.0, "256 mod 16 (exact division)"},
        {12345.0, 97.0, "12345 mod 97 (larger)"},
        
        // Negative numbers
        {-17.0, 5.0, "-17 mod 5 (negative dividend)"},
        {17.0, -5.0, "17 mod -5 (negative divisor)"},
        {-17.0, -5.0, "-17 mod -5 (both negative)"},
        {-100.0, 7.0, "-100 mod 7"},
        {-42.0, 5.0, "-42 mod 5"},
        
        // Edge cases
        {0.0, 5.0, "0 mod 5 (zero dividend)"},
        {5.0, 0.0, "5 mod 0 (zero divisor)"},
        {0.0, 0.0, "0 mod 0 (both zero)"},
        {5.0, 10.0, "5 mod 10 (x < p)"},
        {-5.0, 10.0, "-5 mod 10 (negative, x < p)"},
        
        // Fractions
        {3.14, 1.0, "3.14 mod 1 (fractional)"},
        {0.42, 0.1, "0.42 mod 0.1"},
        {7.5, 2.5, "7.5 mod 2.5 (fractional, exact)"},
        {1.234, 0.01, "1.234 mod 0.01"},
        
        // Large values
        {1e6, 3.0, "1e6 mod 3"},
        {1e9, 97.0, "1e9 mod 97"},
    };
    
    int passed = 0;
    int failed = 0;
    
    std::cout << std::left << std::setw(30) << "Description"
              << std::setw(8) << "x"
              << std::setw(6) << "p"
              << std::setw(16) << "std::fmod"
              << std::setw(16) << "FGG_Mod"
              << std::setw(12) << "Abs Error"
              << "Match\n";
    std::cout << std::string(100, '-') << "\n";
    
    for (auto& t : tests) {
        double expected = std::fmod(t.x, t.p);
        double result = FGG_Mod(t.x, t.p);
        
        double abs_error = std::abs(result - expected);
        bool match = false;
        
        if (std::isnan(expected) && std::isnan(result)) {
            match = true;
        } else if (std::isnan(expected) || std::isnan(result)) {
            match = false;
        } else {
            match = (abs_error < 0.01);
        }
        
        if (match) passed++; else failed++;
        
        std::cout << std::left << std::setw(30) << t.description
                  << std::setw(8) << t.x
                  << std::setw(6) << t.p
                  << std::setw(16) << std::fixed << std::setprecision(6) << expected
                  << std::setw(16) << std::fixed << std::setprecision(6) << result
                  << std::setw(12) << std::fixed << std::setprecision(8) << abs_error
                  << (match ? "OK" : "FAIL") << "\n";
    }
    
    std::cout << "\n  RESULTS: " << passed << "/" << (passed + failed) << " passed";
    if (failed > 0) std::cout << " — " << failed << " FAILED";
    std::cout << "\n\n";
    
    // ======================================================================
    // ANALYSIS: IS FGG_Mod JUST abs()?
    // ======================================================================
    std::cout << "================================================================================\n";
    std::cout << "  ANALYSIS: Is FGG_Mod just computing abs()?\n";
    std::cout << "================================================================================\n\n";
    
    std::cout << "  Test: Compare FGG_Mod(x, p) with fmod(abs(x), p)\n";
    std::cout << "  If FGG_Mod = abs-based fmod, they should match exactly.\n\n";
    
    std::cout << std::left << std::setw(10) << "x"
              << std::setw(6) << "p"
              << std::setw(16) << "FGG_Mod"
              << std::setw(16) << "fmod(abs,p)"
              << std::setw(16) << "fmod(x,p)"
              << "Different from abs?\n";
    std::cout << std::string(75, '-') << "\n";
    
    int different_from_abs = 0;
    for (auto& t : tests) {
        if (std::isnan(std::fmod(t.x, t.p))) continue;
        
        double fgg = FGG_Mod(t.x, t.p);
        double abs_fmod = std::fmod(std::abs(t.x), std::abs(t.p));
        if (t.x < 0 && abs_fmod != 0.0) abs_fmod = abs_fmod - std::abs(t.p);
        double std_fmod = std::fmod(t.x, t.p);
        
        bool differs = (std::abs(fgg - abs_fmod) > 0.01) || 
                       (std::abs(fgg - std_fmod) > 0.01);
        if (differs) different_from_abs++;
        
        if (differs) {
            std::cout << std::left << std::setw(10) << t.x
                      << std::setw(6) << t.p
                      << std::setw(16) << std::fixed << std::setprecision(4) << fgg
                      << std::setw(16) << std::fixed << std::setprecision(4) << abs_fmod
                      << std::setw(16) << std::fixed << std::setprecision(4) << std_fmod
                      << "YES\n";
        }
    }
    
    if (different_from_abs == 0) {
        std::cout << "  (All results match abs-based fmod)\n";
    }
    std::cout << "\n  Cases where FGG_Mod differs from abs-based: " << different_from_abs << "/" << tests.size() << "\n\n";
    
    // ======================================================================
    // THE HONEST CONCLUSION
    // ======================================================================
    std::cout << "================================================================================\n";
    std::cout << "  HONEST CONCLUSION\n";
    std::cout << "================================================================================\n\n";
    
    if (passed == passed + failed) {
        std::cout << "  All tests passed. FGG_Mod matches std::fmod behavior.\n\n";
    } else {
        std::cout << "  " << failed << " test(s) failed. FGG_Mod does NOT fully match std::fmod.\n\n";
    }
    
    if (different_from_abs == 0) {
        std::cout << "  HOWEVER: FGG_Mod produces the same result as fmod(abs(x), p)\n";
        std::cout << "  with sign adjustment. This means FGG_Mod is equivalent to:\n";
        std::cout << "    result = fmod(|x|, |p|)\n";
        std::cout << "    if x < 0 and result != 0: result -= |p|\n\n";
        std::cout << "  The FGG(v,3) = |v| property collapses to absolute value.\n";
        std::cout << "  The modulo behavior comes from the scaling x/p and fractional extraction,\n";
        std::cout << "  not from a novel modular reduction by FGG itself.\n\n";
        std::cout << "  VERDICT: FGG_Mod is an ABS-BASED modulo, not a novel canonical reduction.\n";
        std::cout << "  The FGG contributes the collapse to |v|, not the modulo semantics.\n";
    }
    
    return 0;
}
