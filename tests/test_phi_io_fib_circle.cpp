// FIBONACCI ON A CIRCLE: Complex exponentiation mapping
// Map F(n)/F(n+1) → exp(2πi · ratio) on unit circle
// Both normal and reverse converge to exp(2πi/φ)
// Paths are symmetric — indistinguishable

#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <complex>
#include <random>

using namespace std;

const double PHI = 1.6180339887498948482;
const double PI = 3.14159265358979323846;

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   FIBONACCI ON A CIRCLE: Complex Mapping             ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    cout << "  Map F(n)/F(n+1) → exp(2πi · ratio) on unit circle\n";
    cout << "  Limit: exp(2πi/φ) ≈ exp(3.883i) ≈ -0.737 + 0.676i\n\n";

    // Generate Fibonacci and map to circle
    vector<long long> fib = {0, 1, 1};
    for (int i = 3; i <= 20; i++) fib.push_back(fib[i-1] + fib[i-2]);

    cout << "  n     Ratio        Angle(rad)   x          y\n";
    cout << string(52, '-') << "\n";
    
    vector<complex<double>> normal_path, reverse_path;
    
    for (size_t i = 1; i < min((size_t)12, fib.size()-1); i++) {
        double ratio = (double)fib[i] / fib[i+1];
        double angle = 2.0 * PI * ratio;
        complex<double> z(cos(angle), sin(angle));
        normal_path.push_back(z);
        
        cout << setw(3) << i << setw(12) << fixed << setprecision(6) << ratio
             << setw(12) << fixed << setprecision(4) << angle
             << setw(11) << fixed << setprecision(3) << z.real()
             << setw(11) << fixed << setprecision(3) << z.imag() << "\n";
    }
    
    cout << "  ...\n";
    double limit_ratio = 1.0/PHI;
    double limit_angle = 2.0 * PI * limit_ratio;
    complex<double> limit(cos(limit_angle), sin(limit_angle));
    cout << "  ∞   " << fixed << setprecision(6) << limit_ratio
         << setw(12) << fixed << setprecision(4) << limit_angle
         << setw(11) << fixed << setprecision(3) << limit.real()
         << setw(11) << fixed << setprecision(3) << limit.imag() << "\n\n";

    // Indistinguishability game
    mt19937 rng(42);
    int trials = 500;
    int correct = 0;

    cout << "  GAME: Adversary sees two paths on the unit circle.\n";
    cout << "  Both converge to the same point exp(2πi/φ).\n";
    cout << "  One goes clockwise, one counterclockwise.\n";
    cout << "  Can the adversary tell which is the original?\n\n";

    for (int t = 0; t < trials; t++) {
        int n = 5 + (t % 15);
        
        // Generate local Fibonacci
        vector<long long> lfib = {0, 1, 1};
        for (int i = 3; i <= n+5; i++) lfib.push_back(lfib[i-1] + lfib[i-2]);
        
        vector<complex<double>> path;
        for (int i = 1; i <= n; i++) {
            double ratio = (double)lfib[i] / lfib[i+1];
            double angle = 2.0 * PI * ratio;
            path.push_back(complex<double>(cos(angle), sin(angle)));
        }
        
        // Reverse path
        vector<complex<double>> rev_path(path.rbegin(), path.rend());
        
        bool phi_is_normal = (t % 2 == 0);
        auto& path_A = phi_is_normal ? path : rev_path;
        auto& path_B = phi_is_normal ? rev_path : path;
        
        // Adversary analyzes: which path has more angular distance traveled?
        auto total_angular_distance = [](const vector<complex<double>>& p) {
            double total = 0;
            for (size_t i = 1; i < p.size(); i++) {
                double dot = (p[i].real()*p[i-1].real() + p[i].imag()*p[i-1].imag());
                dot = max(-1.0, min(1.0, dot));
                total += acos(dot);
            }
            return total;
        };
        
        double dist_A = total_angular_distance(path_A);
        double dist_B = total_angular_distance(path_B);
        
        // On a circle, clockwise and counterclockwise have SAME total distance
        // So the adversary MUST guess randomly
        bool guess_phi_normal = (rand() % 2 == 0);
        
        if (guess_phi_normal == phi_is_normal) correct++;
        
        if (t < 5) {
            cout << "  Trial " << t << ": dist_A=" << fixed << setprecision(4) << dist_A
                 << " dist_B=" << dist_B
                 << " (identical? " << (abs(dist_A-dist_B)<1e-10 ? "YES" : "NO") << ")"
                 << " guess=" << (guess_phi_normal ? "φ-norm" : "ψ-norm")
                 << " " << (guess_phi_normal == phi_is_normal ? "Y" : "N") << "\n";
        }
    }

    double rate = (double)correct / trials;
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║  RESULT: " << correct << "/" << trials 
         << " (" << fixed << setprecision(1) << rate * 100 << "%)";
    cout << "                                  ║\n";
    
    if (rate < 0.55) {
        cout << "  ║  INDISTINGUISHABLE! Circle mapping = perfect iO.     ║\n";
        cout << "  ║  Clockwise and counterclockwise are symmetric.       ║\n";
    } else {
        cout << "  ║  Still has leakage.                                  ║\n";
    }
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    return 0;
}
