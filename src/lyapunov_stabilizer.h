#ifndef LYAPUNOV_STABILIZER_H
#define LYAPUNOV_STABILIZER_H

#include <cmath>

constexpr double PHI_STABLE = 1.6180339887498948482;
constexpr double PHI_STABLE_INV = 0.6180339887498948482;
constexpr double LAMBDA_STABLE = 0.4812;

// ═══════════════════════════════════════════
// SELF-REFERENTIAL ATTRACTOR STABILIZATION
// ═══════════════════════════════════════════
// Instead of pulling toward a fixed attractor (40),
// pull toward the φ-weighted midpoint of the inputs.
// This preserves the signal while damping the noise.

class LyapunovStabilizer {
private:
    double current_drift;
    uint64_t operation_count;
    double lyapunov_entropy;
    
public:
    LyapunovStabilizer() : current_drift(0.0), operation_count(0), lyapunov_entropy(0.0) {}
    
    // Stabilize using self-referential attractor
    // attractor = (e1 + e2) / 2  (midpoint of inputs)
    double stabilize(double e_result, double e1, double e2) {
        operation_count++;
        
        // Self-referential attractor: the φ-weighted midpoint
        double e_attractor = (e1 * PHI_STABLE_INV + e2 * (1.0 - PHI_STABLE_INV));
        
        // φ-contraction: pull result slightly toward attractor
        // Use very mild contraction (0.1% toward attractor)
        double contraction_strength = 0.001;  // 0.1%
        double e_stabilized = e_result * (1.0 - contraction_strength) 
                            + e_attractor * contraction_strength;
        
        // Track drift
        current_drift = fabs(e_stabilized - e_result);
        lyapunov_entropy = lyapunov_entropy * PHI_STABLE_INV 
                         + current_drift * (1.0 - PHI_STABLE_INV);
        
        return e_stabilized;
    }
    
    double get_drift() const { return current_drift; }
    double get_entropy() const { return lyapunov_entropy; }
    uint64_t get_op_count() const { return operation_count; }
};

#endif
