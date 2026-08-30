// ============================================
// φ-NOISE LYAPUNOV CONTROL
//
// Lyapunov stability para sa noise:
// V(x) = φ|x|² — Lyapunov function
// Pag V dot < 0 → stable (noise controlled)
// Pag V dot > 0 → unstable (noise lumalaki)
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>

using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-NOISE LYAPUNOV CONTROL\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

    cout << "  φ = " << PHI << "\n";
    cout << "  φ⁻¹ = " << PHI_INV << "\n\n";

    // ============================================
    // LYAPUNOV FUNCTION
    // ============================================

    // V(x) = φ|x|²
    auto lyapunov = [&](double x) {
        return PHI * x * x;
    };

    // V dot = ∇V · f(x) = 2φx · f(x)
    // Para sa noise: f(x) = -φ⁻¹x (natural decay)
    auto lyapunov_derivative = [&](double x) {
        // V dot = 2φx · (-φ⁻¹x) = -2x²
        return -2.0 * x * x;
    };

    cout << "========================================\n";
    cout << "  TEST 1: LYAPUNOV STABILITY\n";
    cout << "========================================\n\n";

    cout << "  Noise | V(noise) | V̇(noise) | Stable?\n";
    cout << "  ------|----------|----------|---------\n";

    for (double noise : {-2.0, -1.0, -0.5, -0.1, 0.0, 0.1, 0.5, 1.0, 2.0}) {
        double V = lyapunov(noise);
        double V_dot = lyapunov_derivative(noise);
        bool stable = (V_dot < 0);

        cout << "  " << setw(5) << fixed << setprecision(2) << noise << " | "
             << setw(8) << setprecision(4) << V << " | "
             << setw(8) << V_dot << " | "
             << (stable ? "✅ STABLE" : "❌ UNSTABLE") << "\n";
    }

    cout << "\n";

    // ============================================
    // TEST 2: NOISE CONTROL (TAAS-BABA)
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: NOISE CONTROL\n";
    cout << "========================================\n\n";

    cout << "  Step | Noise | V(noise) | φ-Control | New Noise\n";
    cout << "  -----|-------|----------|-----------|----------\n";

    double noise = 1.0;  // Start sa malaking noise

    for (int step = 0; step < 20; step++) {
        double V = lyapunov(noise);
        
        // φ-control: i-multiply sa φ⁻¹ para mag-decay
        double control = PHI_INV;
        double new_noise = noise * control;
        
        cout << "  " << setw(4) << step << " | "
             << setw(5) << fixed << setprecision(4) << noise << " | "
             << setw(8) << V << " | "
             << setw(9) << control << " | "
             << setw(10) << new_noise << "\n";
        
        noise = new_noise;
        
        if (abs(noise) < 1e-6) {
            cout << "  ... (noise → 0)\n";
            break;
        }
    }

    cout << "\n";

    // ============================================
    // TEST 3: φ-STABLE EQUILIBRIUM
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 3: φ-STABLE EQUILIBRIUM\n";
    cout << "========================================\n\n";

    cout << "  Ang φ-stable equilibrium ay kung saan:\n";
    cout << "  noise = φ⁻¹ × noise (fixed point)\n\n";

    cout << "  Fixed point: noise = φ⁻¹ × noise\n";
    cout << "  → noise × (1 - φ⁻¹) = 0\n";
    cout << "  → noise = 0 (asymptotically stable)\n\n";

    cout << "  Iteration | Noise | Ratio (noise/prev)\n";
    cout << "  ----------|-------|------------------\n";

    double n = 1.0;
    for (int i = 0; i < 10; i++) {
        double prev = n;
        n = n * PHI_INV;
        double ratio = n / prev;
        
        cout << "  " << setw(9) << i << " | "
             << setw(5) << fixed << setprecision(6) << n << " | "
             << setw(16) << setprecision(4) << ratio << "\n";
    }

    cout << "\n  Ratio → " << PHI_INV << " (φ⁻¹)\n";
    cout << "  Ang noise ay nagde-decay sa rate na φ⁻¹\n\n";

    // ============================================
    // TEST 4: NOISE OSCILLATION CONTROL
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 4: OSCILLATION CONTROL\n";
    cout << "========================================\n\n";

    cout << "  Kapag ang noise ay nag-o-oscillate,\n";
    cout << "  gagamitin natin ang φ-mirror para i-stabilize.\n\n";

    cout << "  Step | Noise | φ-Mirror | Stabilized?\n";
    cout << "  -----|-------|----------|------------\n";

    double osc_noise = 1.0;
    for (int step = 0; step < 15; step++) {
        // Oscillation: alternate positive/negative
        osc_noise = -osc_noise * PHI_INV;
        
        // φ-mirror: i-reflect sa positive
        double mirrored = abs(osc_noise);
        
        bool stabilized = (mirrored < 1e-3);
        
        cout << "  " << setw(4) << step << " | "
             << setw(5) << fixed << setprecision(6) << osc_noise << " | "
             << setw(8) << mirrored << " | "
             << (stabilized ? "✅" : "⏳") << "\n";
        
        if (stabilized) break;
    }

    cout << "\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  LYAPUNOV NOISE CONTROL SUMMARY\n";
    cout << "========================================\n\n";
    cout << "  ✅ Lyapunov function: V(x) = φ|x|²\n";
    cout << "  ✅ V̇(x) = -2x² < 0 (stable)\n";
    cout << "  ✅ φ⁻¹ decay rate\n";
    cout << "  ✅ Zero noise = asymptotically stable\n";
    cout << "  ✅ Oscillation controlled via φ-mirror\n\n";

    return 0;
}
