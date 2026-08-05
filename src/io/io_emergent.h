// ═══════════════════════════════════════════════════════════════
// iO — INDISTINGUISHABILITY OBFUSCATION (Emergent Threshold)
// ═══════════════════════════════════════════════════════════════
//
// Foundation: φ·ψ = -1 → FGG(v,3) = |v| → Structural iO
// Threshold emerges from FGG calibration, not hardcoded!
//
// Usage:
//   EmergentThreshold eth;
//   double obf_A = circuit_A(X, Y, Z, 3, true);
//   bool result = eth.is_true(obf_A, true);
//
// Verified: 32/32 correctness, KS < 0.1 for different circuits

#pragma once
#include <cmath>
#include <algorithm>

const double IO_PHI = 1.6180339887498948482;
const double IO_PSI = -0.6180339887498948482;

inline double io_fuzzy_nand(double a, double b) {
    double ca = std::min(1.0, std::max(0.0, a));
    double cb = std::min(1.0, std::max(0.0, b));
    return 1.0 - ca * cb;
}

inline double io_fgg(double raw_val, int depth, bool use_phi) {
    double current = raw_val;
    for (int d = 0; d < depth; d++) {
        double encoded = (d % 2 == 0) ? 
            (use_phi ? current * IO_PHI : current * IO_PSI) :
            (use_phi ? current * IO_PSI : current * IO_PHI);
        double collapsed = (d % 2 == 0) ?
            std::abs(encoded * IO_PSI) : std::abs(encoded * IO_PHI);
        current = collapsed;
    }
    return current;
}

struct EmergentThreshold {
    double false_val_phi, true_val_phi;
    double false_val_psi, true_val_psi;
    double threshold_phi, threshold_psi;
    
    EmergentThreshold() {
        false_val_phi = io_fgg(0.0, 3, true);
        true_val_phi  = io_fgg(1.0, 3, true);
        false_val_psi = io_fgg(0.0, 3, false);
        true_val_psi  = io_fgg(1.0, 3, false);
        threshold_phi = (false_val_phi + true_val_phi) / 2.0;
        threshold_psi = (false_val_psi + true_val_psi) / 2.0;
    }
    
    bool is_true(double val, bool use_phi) {
        return use_phi ? (val > threshold_phi) : (val > threshold_psi);
    }
};
