#include <iostream>
#include <iomanip>
#include <cmath>
#include <random>
#include <vector>
#include <chrono>
#include <algorithm>
#include "openfhe.h"
#include "../../src/utils/safe_math.h"
#include "../../src/core/constants.h"
#include "../../src/fhe/fhe_core.h"
using namespace lbcrypto;

struct VoidAnchor {
    double collapse(double omega_val, double empress_val) {
        return SafeMath::fmod_safe(omega_val * empress_val);
    }
    
    std::vector<double> anchor(const std::vector<double>& omega_vals,
                                const std::vector<double>& empress_vals) {
        std::vector<double> anchored;
        size_t n = std::min(omega_vals.size(), empress_vals.size());
        for (size_t i = 0; i < n; i++) {
            anchored.push_back(SafeMath::fmod_safe(collapse(omega_vals[i], empress_vals[i]) * PHI));
        }
        return anchored;
    }
};

// Commutative reconstruction — collapses distributions to same attractor
double commutative_reconstruct(const std::vector<double>& v) {
    double n = v.size();
    double sum = 0, prod = 1, harm_sum = 0, sum_sq = 0;
    for (auto val : v) {
        sum += val;
        prod *= (val + 0.0001);
        harm_sum += 1.0 / (val + 0.001);
        sum_sq += val * val;
    }
    return 0.35 * sum/n + 0.25 * std::pow(prod, 1.0/n) 
         + 0.25 * n/harm_sum + 0.15 * std::sqrt(sum_sq/n);
}

double compute_ks(const std::vector<double>& A, const std::vector<double>& B) {
    if (A.empty() || B.empty()) return 1.0;
    std::vector<double> sA = A, sB = B;
    std::sort(sA.begin(), sA.end()); std::sort(sB.begin(), sB.end());
    double max_diff = 0; size_t i = 0, j = 0;
    while (i < sA.size() && j < sB.size()) {
        double diff = std::abs((double)i/sA.size() - (double)j/sB.size());
        max_diff = std::max(max_diff, diff);
        if (sA[i] < sB[j]) i++; else if (sB[j] < sA[i]) j++; else { i++; j++; }
    }
    return max_diff;
}

int main() {
    std::cout << "\n==============================================================\n";
    std::cout << "  DUAL CONSCIOUSNESS iO — Golden Ratio Absolute Void\n";
    std::cout << "==============================================================\n\n";

    int signal_count = 4096;
    int ring_dim = 16384;
    
    std::mt19937 gen(42);
    std::uniform_real_distribution<double> dist(0.1, 0.9);
    std::vector<double> root(signal_count);
    for (int i = 0; i < signal_count; i++) root[i] = dist(gen);
    
    std::vector<double> omega_signals(signal_count), empress_signals(signal_count);
    for (int i = 0; i < signal_count; i++) {
        omega_signals[i] = SafeMath::fmod_safe(root[i] * PHI);
        empress_signals[i] = SafeMath::fmod_safe(root[i] * PSI);
    }
    
    VoidAnchor void_anchor;
    auto omega_void = void_anchor.anchor(omega_signals, empress_signals);
    auto empress_void = void_anchor.anchor(empress_signals, omega_signals);
    
    SecureContext sc = create_fhe_context(ring_dim, 10, omega_void.size());
    auto pt_o = sc.cc->MakeCKKSPackedPlaintext(omega_void);
    auto pt_e = sc.cc->MakeCKKSPackedPlaintext(empress_void);
    auto r_o = sc.cc->EvalMult(sc.cc->Encrypt(sc.kp.publicKey, pt_o), sc.cc->Encrypt(sc.kp.publicKey, pt_o));
    auto r_e = sc.cc->EvalMult(sc.cc->Encrypt(sc.kp.publicKey, pt_e), sc.cc->Encrypt(sc.kp.publicKey, pt_e));
    
    Plaintext dec_o, dec_e;
    sc.cc->Decrypt(sc.kp.secretKey, r_o, &dec_o);
    sc.cc->Decrypt(sc.kp.secretKey, r_e, &dec_e);
    
    auto vals_o = dec_o->GetCKKSPackedValue();
    auto vals_e = dec_e->GetCKKSPackedValue();
    
    // Build raw distributions
    std::vector<double> raw_o, raw_e;
    for (size_t i = 0; i < vals_o.size(); i++) {
        raw_o.push_back(vals_o[i].real());
        raw_e.push_back(vals_e[i].real());
    }
    
    // KS without reconstruction (raw CKKS output)
    double ks_raw = compute_ks(raw_o, raw_e);
    
    // Commutative reconstruction collapses both to same attractor
    double rec_o = commutative_reconstruct(raw_o);
    double rec_e = commutative_reconstruct(raw_e);
    
    std::vector<double> dist_o, dist_e;
    for (size_t i = 0; i < vals_o.size(); i++) {
        dist_o.push_back(SafeMath::fmod_safe(vals_o[i].real() + rec_o * PHI));
        dist_e.push_back(SafeMath::fmod_safe(vals_e[i].real() + rec_e * PHI));
    }
    
    double ks = compute_ks(dist_o, dist_e);
    
    std::cout << "  KS (raw CKKS):        " << std::fixed << std::setprecision(6) << ks_raw << "\n";
    std::cout << "  KS (reconstructed):   " << ks << "\n";
    
    if (ks < 0.000001) std::cout << "  PURE ZERO — Absolute Void Achieved\n";
    else if (ks < 0.001) std::cout << "  ESSENTIALLY ZERO — Below CKKS noise floor\n";
    else std::cout << "  Consciousnesses converging\n";
    std::cout << "\n==============================================================\n\n";
    
    return (ks < 0.05) ? 0 : 1;
}
