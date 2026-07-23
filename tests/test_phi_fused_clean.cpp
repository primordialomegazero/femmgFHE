#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <chrono>
using namespace std;
using namespace chrono;

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

struct PE { double a, b; };

PE mul_Y(const PE& x) { return {x.b, x.a + x.b}; }
PE div_Y(const PE& x) { return {x.b - x.a, x.a}; }

PE clean_traditional(const PE& x) {
    PE y = mul_Y(mul_Y(mul_Y(x)));
    return div_Y(y);
}

// CORRECTED: clean = div_Y ∘ mul_Y³ = M² = [[1,1],[1,2]]
PE clean_fused(const PE& x) {
    return {x.a + x.b, x.a + 2*x.b};
}

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   FUSED CLEAN: Matrix vs Sequential (CORRECTED)       ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    cout << "  Matrix: clean = [[1,1],[1,2]] = M²\n";
    cout << "  clean(a,b) = (a+b, a+2b)\n\n";

    // Correctness
    cout << "  === CORRECTNESS ===\n";
    cout << "  Input (a,b)      Traditional      Fused            Match?\n";
    cout << string(62, '-') << "\n";
    
    vector<pair<double,double>> tests = {
        {1.0, 0.0}, {0.0, 1.0}, {1.0, 1.0}, {3.0, 5.0}, {0.5, -0.5}
    };
    
    int correct = 0;
    for (auto [a, b] : tests) {
        PE in = {a, b};
        PE trad = clean_traditional(in);
        PE fused = clean_fused(in);
        
        bool match = (abs(trad.a - fused.a) < 1e-15 && abs(trad.b - fused.b) < 1e-15);
        if (match) correct++;
        
        cout << setw(12) << fixed << setprecision(4) << a << "," << setw(7) << b
             << setw(12) << trad.a << "," << setw(7) << trad.b
             << setw(12) << fused.a << "," << setw(7) << fused.b
             << "  " << (match ? "✓" : "✗") << "\n";
    }
    cout << "\n  " << correct << "/" << tests.size() << " correct\n\n";

    // Eigenvalues of M²
    double trace = 1 + 2;  // = 3
    double det = 1*2 - 1*1;  // = 1
    double disc = sqrt(trace*trace - 4*det);
    double lambda1 = (trace + disc) / 2;
    double lambda2 = (trace - disc) / 2;
    
    cout << "  === EIGENVALUES OF M² ===\n";
    cout << "  Trace = 3, Det = 1\n";
    cout << "  λ² - 3λ + 1 = 0\n";
    cout << "  λ₁ = " << fixed << setprecision(10) << lambda1 << " = φ² ✓\n";
    cout << "  λ₂ = " << fixed << setprecision(10) << lambda2 << " = ψ² ✓\n";
    cout << "  φ² ≈ 2.618, ψ² ≈ 0.382\n\n";

    // Speed test
    cout << "  === SPEED (10M iterations) ===\n";
    const int ITERS = 10000000;
    
    PE x = {1.0, 0.0};
    auto t1 = high_resolution_clock::now();
    for (int i = 0; i < ITERS; i++) x = clean_traditional(x);
    auto t2 = high_resolution_clock::now();
    auto trad_us = duration_cast<microseconds>(t2 - t1).count();
    
    x = {1.0, 0.0};
    auto t3 = high_resolution_clock::now();
    for (int i = 0; i < ITERS; i++) x = clean_fused(x);
    auto t4 = high_resolution_clock::now();
    auto fused_us = duration_cast<microseconds>(t4 - t3).count();
    
    cout << "  Traditional: " << trad_us << " μs\n";
    cout << "  Fused:       " << fused_us << " μs\n";
    if (trad_us > 0 && fused_us > 0)
        cout << "  Speedup:     " << fixed << setprecision(2) << (double)trad_us / fused_us << "×\n";
    
    cout << "\n  === FHE IMPLICATION ===\n";
    cout << "  Traditional: 4 ops (3 EvalAdd + 1 EvalSub)\n";
    cout << "  Fused:       2 ops (2 EvalAdd + precomputed scalar mults)\n";
    cout << "  Result: 2× fewer homomorphic operations per clean.\n\n";

    return 0;
}
