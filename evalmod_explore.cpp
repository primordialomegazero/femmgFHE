#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdint>

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

/**
 * Attempt: Can FGG directly compute modulo?
 * 
 * Hypothesis: FGG(v,3) applied to a φ-weighted value
 * with the modulus embedded in the weighting creates
 * a natural collapse to the canonical residue.
 */

// Standard FGG
double FGG(double v, int depth = 3) {
    double current = v;
    for (int d = 0; d < depth; d++) {
        if (d % 2 == 0) {
            current = fabs((current * PHI) * PSI);
        } else {
            current = fabs((current * PSI) * PHI);
        }
    }
    return fabs(current);
}

// Attempt 1: Direct FGG on x, then scale by n
int64_t evalmod_v1(int64_t x, int64_t n) {
    double scaled = (double)x / (double)n;
    double collapsed = FGG(scaled, 3);
    double result = collapsed * (double)n;
    return (int64_t)round(result) % n;
}

// Attempt 2: Encode with modulus in the phase
int64_t evalmod_v2(int64_t x, int64_t n) {
    double phase = (double)x * PHI / (double)n;
    double collapsed = FGG(phase, 3);
    double result = (collapsed / PHI) * (double)n;
    int64_t out = (int64_t)round(result) % n;
    return ((out % n) + n) % n;
}

// Attempt 3: Use PSI as the modular base
int64_t evalmod_v3(int64_t x, int64_t n) {
    double v = fmod((double)x * PHI, (double)n);
    double collapsed = FGG(v, 3);
    return ((int64_t)round(collapsed) % n + n) % n;
}

int main() {
    std::cout << "\n";
    std::cout << "================================================================================\n";
    std::cout << "  EvalMod EXPLORATION — Can FGG Directly Compute Modulo?\n";
    std::cout << "================================================================================\n\n";

    struct Test { int64_t x, n; };
    Test tests[] = {
        {17, 5}, {42, 7}, {100, 13}, {-17, 5}, {12345, 97}
    };

    std::cout << std::left << std::setw(8) << "x" << std::setw(6) << "n"
              << std::setw(10) << "Expected" << std::setw(10) << "V1"
              << std::setw(10) << "V2" << std::setw(10) << "V3" << "\n";
    std::cout << std::string(50, '-') << "\n";

    for (auto& t : tests) {
        int64_t exp = ((t.x % t.n) + t.n) % t.n;
        std::cout << std::left << std::setw(8) << t.x << std::setw(6) << t.n
                  << std::setw(10) << exp
                  << std::setw(10) << evalmod_v1(t.x, t.n)
                  << std::setw(10) << evalmod_v2(t.x, t.n)
                  << std::setw(10) << evalmod_v3(t.x, t.n) << "\n";
    }

    // Test all three methods systematically
    std::cout << "\n  Systematic test (n=5, all residues):\n  ";
    for (int x = 0; x < 25; x++) {
        int expected = x % 5;
        int v1 = evalmod_v1(x, 5);
        int v2 = evalmod_v2(x, 5);
        int v3 = evalmod_v3(x, 5);
        
        if (x < 10 || x > 20) {
            std::cout << "x=" << x << ": exp=" << expected 
                      << " v1=" << v1 << " v2=" << v2 << " v3=" << v3;
            if (v1 == expected && v2 == expected && v3 == expected)
                std::cout << " ALL OK";
            std::cout << "\n  ";
        }
    }

    std::cout << "\n  Verdict: ";
    bool any_works = false;
    for (int x = 0; x < 100; x++) {
        if (evalmod_v1(x, 5) == x % 5) any_works = true;
    }
    std::cout << (any_works ? "SOME METHODS SHOW PROMISE" : "NONE WORK DIRECTLY") << "\n";
    std::cout << "  FGG is identity on encoded values, not modular reduction.\n";
    std::cout << "  True EvalMod requires non-polynomial approximation.\n\n";

    return 0;
}
