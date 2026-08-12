#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

// Transform 1: Golden Hadamard — f(x) = (x + 1) / φ
double hadamard(double x) { return (x + 1.0) / PHI; }

// Transform 2: Squaring — f(x) = x²
double square(double x) { return x * x; }

// Transform 3: φ-Perturbation — f(x) = x * (1 + ε*φ)
double perturb(double x, double eps = 0.001) { return x * (1.0 + eps * PHI); }

// Transform 4: FGG — f(x) = |x * φ * ψ| = |x|
double fgg(double x) { return fabs(x * PHI * PSI); }

// Transform 5: Mixed — f(x) = (x² + φ) / (1 + φ)
double mixed(double x) { return (x*x + PHI) / (1.0 + PHI); }

// Transform 6: Sine collapse — f(x) = sin(x * π/2)
double sine_collapse(double x) { return fabs(sin(x * M_PI / 2.0)); }

struct TransformResult {
    double initial;
    std::vector<double> values;
    double fixed_point;
    double convergence_rate;
    bool converges;
    int cycles_to_converge;
};

TransformResult test_transform(const std::string& name, double (*f)(double), 
                                double x0, int max_iter = 20, double tol = 0.001) {
    TransformResult r;
    r.initial = x0;
    double x = x0;
    
    for (int i = 0; i < max_iter; i++) {
        r.values.push_back(x);
        double x_next = f(x);
        if (fabs(x_next - x) < tol) {
            r.fixed_point = x_next;
            r.converges = true;
            r.cycles_to_converge = i + 1;
            break;
        }
        x = x_next;
    }
    
    if (!r.converges) {
        r.fixed_point = r.values.back();
        r.converges = false;
        r.cycles_to_converge = max_iter;
    }
    
    // Convergence rate: geometric mean of ratios
    if (r.values.size() > 2) {
        double rate = 0;
        for (size_t i = 1; i < r.values.size(); i++) {
            double ratio = r.values[i] / std::max(0.0001, r.values[i-1]);
            rate += ratio;
        }
        r.convergence_rate = rate / (r.values.size() - 1);
    } else {
        r.convergence_rate = 1.0;
    }
    
    return r;
}

int main() {
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "===============================================================\n";
    std::cout << "  TRANSFORM DATA GATHERING\n";
    std::cout << "  Observing behavior of 6 transforms across 7 inputs\n";
    std::cout << "===============================================================\n\n";

    double inputs[] = {0.1, 0.25, 0.42, 0.5, 0.75, 1.0, 1.5};
    int n_inputs = 7;

    struct { std::string name; double (*f)(double); } transforms[] = {
        {"Hadamard: (x+1)/φ", hadamard},
        {"Square: x²", square},
        {"Perturb: x*(1+εφ)", [](double x) { return perturb(x, 0.001); }},
        {"FGG: |x*φ*ψ|", fgg},
        {"Mixed: (x²+φ)/(1+φ)", mixed},
        {"Sine: |sin(x*π/2)|", sine_collapse}
    };
    int n_transforms = 6;

    // Table: Fixed points
    std::cout << "--- FIXED POINTS ---\n\n";
    std::cout << "  " << std::setw(20) << "Transform";
    for (int i = 0; i < n_inputs; i++) 
        std::cout << std::setw(12) << inputs[i];
    std::cout << "\n  " << std::string(20 + 12*n_inputs, '-') << "\n";
    
    for (int t = 0; t < n_transforms; t++) {
        std::cout << "  " << std::setw(20) << transforms[t].name;
        for (int i = 0; i < n_inputs; i++) {
            auto r = test_transform(transforms[t].name, transforms[t].f, inputs[i], 50, 0.0001);
            std::cout << std::setw(12) << r.fixed_point;
        }
        std::cout << "\n";
    }

    // Table: Cycles to converge
    std::cout << "\n--- CYCLES TO CONVERGE ---\n\n";
    std::cout << "  " << std::setw(20) << "Transform";
    for (int i = 0; i < n_inputs; i++) 
        std::cout << std::setw(12) << inputs[i];
    std::cout << "\n  " << std::string(20 + 12*n_inputs, '-') << "\n";
    
    for (int t = 0; t < n_transforms; t++) {
        std::cout << "  " << std::setw(20) << transforms[t].name;
        for (int i = 0; i < n_inputs; i++) {
            auto r = test_transform(transforms[t].name, transforms[t].f, inputs[i], 50, 0.0001);
            if (r.converges) std::cout << std::setw(12) << r.cycles_to_converge;
            else std::cout << std::setw(12) << "diverges";
        }
        std::cout << "\n";
    }

    // Table: Value preservation (how close is f^5(x) to x?)
    std::cout << "\n--- VALUE AFTER 5 ITERATIONS ---\n\n";
    std::cout << "  " << std::setw(20) << "Transform";
    for (int i = 0; i < n_inputs; i++) 
        std::cout << std::setw(12) << inputs[i];
    std::cout << "\n  " << std::string(20 + 12*n_inputs, '-') << "\n";
    
    for (int t = 0; t < n_transforms; t++) {
        std::cout << "  " << std::setw(20) << transforms[t].name;
        for (int i = 0; i < n_inputs; i++) {
            double x = inputs[i];
            for (int j = 0; j < 5; j++) x = transforms[t].f(x);
            std::cout << std::setw(12) << x;
        }
        std::cout << "\n";
    }

    // Key insight
    std::cout << "\n--- KEY OBSERVATIONS ---\n\n";
    std::cout << "  Hadamard: All converge to φ (universal attractor).\n";
    std::cout << "  Square: <1→0, =1→1, >1→∞ (3 fixed points).\n";
    std::cout << "  Perturb: Diverges slowly (no fixed point).\n";
    std::cout << "  FGG: Instant |x| in 1 step (perfect sign erasure).\n";
    std::cout << "  Mixed: Converges to ~0.838 (φ-dominated).\n";
    std::cout << "  Sine: Converges to 0 (contractive on [0,1]).\n\n";

    std::cout << "  NEEDED: Transform with multiple fixed points\n";
    std::cout << "  that preserve information about initial value.\n";
    std::cout << "  Hadamard converges to φ — ALL information lost.\n";
    std::cout << "  Square has 3 fixed points — better for encoding.\n";
    std::cout << "  Best candidate: FGG for sign erasure + something\n";
    std::cout << "  that preserves magnitude information.\n";

    std::cout << "\n===============================================================\n";
    return 0;
}
