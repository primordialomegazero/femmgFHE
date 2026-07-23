// PHI-IO FIBONACCI PARALLEL: Normal vs Reverse Fibonacci
// Both compute F(N) correctly, but through opposite paths
// φ-reality: normal Fibonacci (1,1,2,3,5,8,...)
// ψ-reality: reverse Fibonacci (starting from F(N), working backwards)
// Both produce SAME output. Intermediate states are COMPLETELY different.
// The adversary sees both patterns — can they tell which is "real"?

#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <random>

using namespace std;

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

// Compute Fibonacci numbers
vector<long long> fib_sequence(int n) {
    vector<long long> f = {0, 1};
    for (int i = 2; i <= n; i++) {
        f.push_back(f[i-1] + f[i-2]);
    }
    return f;
}

// Normal Fibonacci: forward computation
// Returns: (intermediate_states, final_result)
struct CircuitTrace {
    vector<double> phi_trace;  // Intermediate values in φ-reality
    vector<double> psi_trace;  // Intermediate values in ψ-reality
    double final_output;
};

CircuitTrace normal_fib(int n) {
    CircuitTrace trace;
    double a = 0, b = 1;
    trace.phi_trace.push_back(a);
    trace.phi_trace.push_back(b);
    
    for (int i = 2; i <= n; i++) {
        double c = a + b;
        trace.phi_trace.push_back(c);
        a = b; b = c;
    }
    trace.final_output = b;
    
    // ψ-reality: reverse Fibonacci
    // Start from F(n) and work backwards using F(k-1) = F(k+1) - F(k)
    double fn = b;
    double fn1 = a + b;  // F(n+1)
    trace.psi_trace.push_back(fn);
    
    a = fn; b = fn1;
    for (int i = n; i >= 1; i--) {
        double prev = b - a;  // F(k-1) = F(k+1) - F(k)
        trace.psi_trace.push_back(prev);
        b = a; a = prev;
    }
    // Reverse the psi trace so indices align (for comparison)
    // Actually keep it reversed — that's the point! Different order!
    
    return trace;
}

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   FIBONACCI PARALLEL: Normal vs Reverse Obfuscation  ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    int N = 10;  // Compute F(10) = 55
    auto trace = normal_fib(N);
    
    cout << "  N = " << N << ", F(" << N << ") = " << trace.final_output << "\n\n";
    
    cout << "  NORMAL FIBONACCI (φ-reality candidate):\n  ";
    for (size_t i = 0; i < trace.phi_trace.size(); i++) {
        cout << setw(4) << fixed << setprecision(0) << trace.phi_trace[i];
    }
    cout << "\n\n";
    
    cout << "  REVERSE FIBONACCI (ψ-reality candidate):\n  ";
    for (size_t i = 0; i < min(trace.psi_trace.size(), trace.phi_trace.size()); i++) {
        cout << setw(4) << fixed << setprecision(0) << trace.psi_trace[i];
    }
    cout << "\n\n";
    
    cout << "  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   INDISTINGUISHABILITY GAME                          ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    
    cout << "  Adversary sees: φ-trace and ψ-trace (one normal, one reverse)\n";
    cout << "  Both produce F(10) = 55. Intermediate values differ.\n";
    cout << "  Can the adversary tell which is which?\n\n";

    mt19937 rng(42);
    int trials = 200;
    int correct = 0;

    cout << "  Trial  F(N)  φ-final  ψ-final  φ-trace[3]  ψ-trace[3]  Guess  OK?\n";
    cout << string(78, '-') << "\n";

    for (int t = 0; t < trials; t++) {
        int n = 5 + (t % 15);  // Vary N from 5 to 19
        auto tr = normal_fib(n);
        
        bool phi_is_normal = (t % 2 == 0);
        
        // The adversary sees these two traces (but doesn't know which is which)
        auto& trace_A = phi_is_normal ? tr.phi_trace : tr.psi_trace;
        auto& trace_B = phi_is_normal ? tr.psi_trace : tr.phi_trace;
        
        // Adversary strategy: normal Fibonacci is monotonically increasing
        // Reverse Fibonacci is monotonically decreasing (when read forward)
        bool A_increasing = true;
        for (size_t i = 1; i < min((size_t)5, trace_A.size()); i++) {
            if (trace_A[i] < trace_A[i-1]) { A_increasing = false; break; }
        }
        bool B_increasing = true;
        for (size_t i = 1; i < min((size_t)5, trace_B.size()); i++) {
            if (trace_B[i] < trace_B[i-1]) { B_increasing = false; break; }
        }
        
        // Guess: the increasing one is normal Fibonacci
        bool guess_phi_normal = A_increasing;
        if (A_increasing == B_increasing) guess_phi_normal = (rand() % 2 == 0);
        
        if (guess_phi_normal == phi_is_normal) correct++;
        
        if (t < 8 || t >= trials - 3) {
            cout << setw(5) << t << setw(6) << n
                 << setw(9) << fixed << setprecision(0) << tr.final_output
                 << setw(9) << fixed << tr.final_output
                 << setw(12) << fixed << setprecision(0) << trace_A[min((size_t)3, trace_A.size()-1)]
                 << setw(12) << fixed << setprecision(0) << trace_B[min((size_t)3, trace_B.size()-1)]
                 << "  " << (guess_phi_normal ? "norm" : "rev ")
                 << "  " << (guess_phi_normal == phi_is_normal ? "Y" : "N") << "\n";
        }
    }

    double rate = (double)correct / trials;
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║  RESULT: " << correct << "/" << trials 
         << " (" << fixed << setprecision(1) << rate * 100 << "%)";
    cout << "                                  ║\n";
    
    if (rate < 0.55) {
        cout << "  ║  INDISTINGUISHABLE. Parallel Fibonacci hides truth.  ║\n";
        cout << "  ║  The two directions are symmetric.                   ║\n";
    } else if (rate < 0.70) {
        cout << "  ║  WEAKLY DISTINGUISHABLE. Need to mask direction.     ║\n";
    } else {
        cout << "  ║  DISTINGUISHABLE. Direction leaks.                   ║\n";
        cout << "  ║  Solution: Randomize intermediate steps.             ║\n";
    }
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    return 0;
}
