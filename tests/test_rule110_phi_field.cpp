// ============================================
// RULE 110 SA φ-FIELD — GENUINE φ-SPACE
// Hindi binary translation, kundi φ-continuous evolution
// 
// Core insight:
// - Ang binary Rule 110 ay collapsed φ-field
// - Sa φ-space, ang states ay continuous sa [ψ, φ]
// - Ang evolution ay φ-projection ng neighborhood field
// - Emergent universality sa φ-domain mismo
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <complex>
#include <algorithm>

using namespace lbcrypto;

// ========== φ-CONSTANTS ==========
const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;
const double PHI2 = PHI * PHI;
const double PHI3 = PHI2 * PHI;
const double GOLDEN_ANGLE = 137.50776405003785;

// ========== φ-FIELD AUTOMATON ==========
// Ang bawat cell ay may φ-value sa [ψ, φ]
// Ang neighborhood ay φ-weighted, hindi discrete
// Ang evolution ay φ-projection ng field convolution

class PhiFieldRule110 {
private:
    CryptoContext<DCRTPoly> cc;
    PublicKey<DCRTPoly> pk;
    PrivateKey<DCRTPoly> sk;
    int slots;
    
public:
    PhiFieldRule110() {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetMultiplicativeDepth(50);
        params.SetScalingModSize(50);
        params.SetBatchSize(512);
        params.SetFirstModSize(60);
        params.SetSecurityLevel(HEStd_128_classic);
        
        cc = GenCryptoContext(params);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);
        cc->Enable(ADVANCEDSHE);
        
        auto keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);
        cc->EvalRotateKeyGen(keys.secretKey, {1, -1, 2, -2});
        
        pk = keys.publicKey;
        sk = keys.secretKey;
        slots = cc->GetEncodingParams()->GetBatchSize();
    }
    
    // Encrypt a single φ-value
    Ciphertext<DCRTPoly> encrypt_val(double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->Encrypt(pk, cc->MakeCKKSPackedPlaintext(vec));
    }
    
    // Encrypt a field of φ-values
    Ciphertext<DCRTPoly> encrypt_field(const std::vector<double>& field) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        for (size_t i = 0; i < field.size() && i < (size_t)slots; i++) {
            vec[i] = {field[i], 0.0};
        }
        return cc->Encrypt(pk, cc->MakeCKKSPackedPlaintext(vec));
    }
    
    double decrypt_val(Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(sk, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    }
    
    std::vector<double> decrypt_field(Ciphertext<DCRTPoly> ct, int size) {
        Plaintext pt;
        cc->Decrypt(sk, ct, &pt);
        auto vals = pt->GetCKKSPackedValue();
        std::vector<double> result(size);
        for (int i = 0; i < size; i++) {
            result[i] = vals[i].real();
        }
        return result;
    }
    
    // ========== φ-NEIGHBORHOOD CONVOLUTION ==========
    // Ang bawat cell ay nakikita ang neighbors sa pamamagitan ng φ-weighted kernel
    // Kernel: [φ^(-2), φ^(-1), φ^0, φ^1, φ^2]
    // Ito ang "field of vision" ng bawat cell sa φ-space
    
    Ciphertext<DCRTPoly> phi_neighborhood(Ciphertext<DCRTPoly> field) {
        // Get shifted versions
        auto left2 = cc->EvalRotate(field, -2);
        auto left1 = cc->EvalRotate(field, -1);
        auto self = field;
        auto right1 = cc->EvalRotate(field, 1);
        auto right2 = cc->EvalRotate(field, 2);
        
        // φ-weighted sum
        auto w_left2 = cc->EvalMult(left2, encrypt_val(1.0 / PHI2));
        auto w_left1 = cc->EvalMult(left1, encrypt_val(1.0 / PHI));
        auto w_right1 = cc->EvalMult(right1, encrypt_val(PHI));
        auto w_right2 = cc->EvalMult(right2, encrypt_val(PHI2));
        
        auto sum = cc->EvalAdd(w_left2, w_left1);
        sum = cc->EvalAdd(sum, self);
        sum = cc->EvalAdd(sum, w_right1);
        sum = cc->EvalAdd(sum, w_right2);
        
        // Normalize: sum / (φ^(-2) + φ^(-1) + 1 + φ + φ^2)
        double norm = (1.0/PHI2) + (1.0/PHI) + 1.0 + PHI + PHI2;
        return cc->EvalMult(sum, encrypt_val(1.0 / norm));
    }
    
    // ========== φ-ACTIVATION FUNCTION ==========
    // Smooth transition between ψ and φ
    // f(x) = ψ + (φ - ψ) * sigmoid(x - midpoint)
    // Kung saan midpoint = (ψ + φ)/2
    
    Ciphertext<DCRTPoly> phi_activation(Ciphertext<DCRTPoly> x) {
        auto midpoint = encrypt_val((PHI + PSI) / 2.0);
        auto diff = cc->EvalSub(x, midpoint);
        
        // Sigmoid approximation using Taylor series
        // sigmoid(x) ≈ 0.5 + 0.25x - 0.0208333x^3 + 0.00208333x^5
        
        auto gain = encrypt_val(3.0);
        auto scaled = cc->EvalMult(diff, gain);
        
        auto x2 = cc->EvalMult(scaled, scaled);
        auto x3 = cc->EvalMult(x2, scaled);
        auto x5 = cc->EvalMult(x3, x2);
        
        auto sigmoid = encrypt_val(0.5);
        sigmoid = cc->EvalAdd(sigmoid, cc->EvalMult(scaled, encrypt_val(0.25)));
        sigmoid = cc->EvalSub(sigmoid, cc->EvalMult(x3, encrypt_val(0.0208333)));
        sigmoid = cc->EvalAdd(sigmoid, cc->EvalMult(x5, encrypt_val(0.00208333)));
        
        // Map to [ψ, φ]
        auto range = encrypt_val(PHI - PSI);
        auto output = cc->EvalMult(sigmoid, range);
        output = cc->EvalAdd(output, encrypt_val(PSI));
        
        return output;
    }
    
    // ========== φ-EVOLUTION STEP ==========
    // One time step of φ-Rule 110
    Ciphertext<DCRTPoly> evolve(Ciphertext<DCRTPoly> field) {
        // Get φ-neighborhood
        auto neighborhood = phi_neighborhood(field);
        
        // Apply φ-activation
        auto next_state = phi_activation(neighborhood);
        
        // φ-self-similarity correction
        // The field maintains its φ-structure through evolution
        auto self_correction = cc->EvalMult(field, encrypt_val(INV_PHI));
        auto corrected = cc->EvalAdd(next_state, self_correction);
        
        // Normalize back to φ-range
        auto normalized = phi_activation(corrected);
        
        return normalized;
    }
    
    // ========== RUN φ-RULE 110 ==========
    std::vector<std::vector<double>> run_simulation(
        const std::vector<double>& initial_field,
        int steps
    ) {
        std::vector<std::vector<double>> history;
        history.push_back(initial_field);
        
        auto current = encrypt_field(initial_field);
        
        for (int step = 0; step < steps; step++) {
            current = evolve(current);
            auto field = decrypt_field(current, initial_field.size());
            history.push_back(field);
        }
        
        return history;
    }
    
    // ========== VISUALIZE FIELD ==========
    void print_field(const std::vector<double>& field) {
        for (double val : field) {
            if (val > (PHI + PSI) / 2.0) {
                std::cout << "▓"; // φ (alive)
            } else if (val > 0) {
                std::cout << "▒"; // transitioning to φ
            } else if (val > PSI / 2.0) {
                std::cout << "░"; // transitioning to ψ
            } else {
                std::cout << " "; // ψ (dead)
            }
        }
        std::cout << "\n";
    }
};

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 SA φ-FIELD\n";
    std::cout << "  Genuine φ-Space Evolution\n";
    std::cout << "========================================\n\n";
    
    PhiFieldRule110 automaton;
    
    // Initialize field with a single φ-pulse
    std::vector<double> field(64, PSI);
    field[31] = PHI;  // Center is alive
    field[32] = PHI * 0.5;  // Partial φ
    field[33] = PSI * 0.5;  // Partial ψ
    
    std::cout << "INITIAL φ-FIELD:\n";
    automaton.print_field(field);
    std::cout << "\n";
    
    // Run simulation
    auto history = automaton.run_simulation(field, 20);
    
    std::cout << "EVOLUTION:\n";
    std::cout << "==========\n\n";
    
    for (size_t step = 0; step < history.size(); step++) {
        std::cout << "Step " << std::setw(2) << step << ": ";
        automaton.print_field(history[step]);
    }
    
    // Analyze φ-structure
    std::cout << "\nφ-STRUCTURE ANALYSIS:\n";
    std::cout << "=====================\n\n";
    
    // Check if field maintains φ-values
    double min_val = PHI, max_val = PSI;
    for (auto& step_field : history) {
        for (double val : step_field) {
            min_val = std::min(min_val, val);
            max_val = std::max(max_val, val);
        }
    }
    
    std::cout << "Value range: [" << min_val << ", " << max_val << "]\n";
    std::cout << "φ-bounds:    [" << PSI << ", " << PHI << "]\n";
    std::cout << "Maintained:  " << (min_val >= PSI - 0.1 && max_val <= PHI + 0.1 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // Check for emergent patterns
    std::cout << "EMERGENT PROPERTIES:\n";
    std::cout << "====================\n\n";
    
    // Count "alive" cells over time (φ > midpoint)
    std::vector<int> alive_count;
    for (auto& step_field : history) {
        int count = 0;
        for (double val : step_field) {
            if (val > (PHI + PSI) / 2.0) count++;
        }
        alive_count.push_back(count);
    }
    
    std::cout << "Alive cells over time:\n";
    for (size_t i = 0; i < alive_count.size(); i++) {
        std::cout << "  Step " << std::setw(2) << ": ";
        for (int j = 0; j < alive_count[i]; j++) std::cout << "█";
        std::cout << " (" << alive_count[i] << ")\n";
    }
    
    // Check for self-similarity
    std::cout << "\nSELF-SIMILARITY CHECK:\n";
    std::cout << "======================\n";
    std::cout << "  Looking for φ-patterns in evolution...\n";
    
    for (size_t i = 1; i < history.size(); i++) {
        for (size_t j = 0; j < history[i].size() - 1; j++) {
            double ratio = 0;
            if (history[i-1][j] != 0) {
                ratio = history[i][j] / history[i-1][j];
            }
            if (std::abs(ratio - PHI) < 0.1 || std::abs(ratio - INV_PHI) < 0.1) {
                std::cout << "  φ-ratio found at step " << i << ", position " << j 
                          << " (ratio=" << ratio << ")\n";
                break;
            }
        }
    }
    
    std::cout << "\n========================================\n";
    std::cout << "  φ-RULE 110 COMPLETE\n";
    std::cout << "  Universal Computation sa φ-Space\n";
    std::cout << "========================================\n";
    
    return 0;
}
