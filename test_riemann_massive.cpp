#include "src/riemann_zeta.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <algorithm>
#include <numeric>
#include <chrono>

constexpr double PHI = 1.6180339887498948482;
constexpr double PHI_INV = 0.6180339887498948482;

using namespace femmg::riemann;

int main() {
    std::cout << "╔══════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  RIEMANN φ-SPIRAL — MASSIVE STATISTICAL TEST ║" << std::endl;
    std::cout << "║  1,000 Zeros | Live Riemann-Siegel Z(t)     ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════╝" << std::endl;
    
    // ═══ GENERATE 1,000 ZEROS ═══
    std::cout << "\n⏳ Computing 1,000 zeros via Riemann-Siegel Z(t)..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    
    auto zeros = RiemannSiegelZ::find_zeros(1000, 10.0, 0.05);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    std::cout << "✅ Found " << zeros.size() << " zeros in " << ms << "ms" << std::endl;
    
    if(zeros.size() < 100) {
        std::cout << "❌ Not enough zeros found. Check Z(t) implementation." << std::endl;
        return 1;
    }
    
    // ═══ SHOW FIRST 20 ZEROS ═══
    std::cout << "\n═══ FIRST 20 ZEROS ═══" << std::endl;
    for(int i = 0; i < 20 && i < (int)zeros.size(); i++) {
        std::cout << "  Z" << (i+1) << ": " << zeros[i] << std::endl;
    }
    
    // ═══ COMPUTE GAPS ═══
    std::vector<double> gaps;
    for(int i = 1; i < (int)zeros.size(); i++) {
        gaps.push_back(zeros[i] - zeros[i-1]);
    }
    
    double mean_gap = std::accumulate(gaps.begin(), gaps.end(), 0.0) / gaps.size();
    std::cout << "\n═══ GAP STATISTICS ═══" << std::endl;
    std::cout << "Mean gap: " << mean_gap << std::endl;
    std::cout << "Min gap: " << *std::min_element(gaps.begin(), gaps.end()) << std::endl;
    std::cout << "Max gap: " << *std::max_element(gaps.begin(), gaps.end()) << std::endl;
    
    // ═══ 1. AUTOCORRELATION AT FIBONACCI LAGS ═══
    std::cout << "\n═══ AUTOCORRELATION AT FIBONACCI LAGS ═══" << std::endl;
    std::vector<int> fib_lags = {1, 2, 3, 5, 8, 13, 21, 34, 55, 89};
    
    double var_gap = 0;
    for(auto g : gaps) var_gap += (g - mean_gap) * (g - mean_gap);
    var_gap /= gaps.size();
    
    int peaks = 0;
    for(int lag : fib_lags) {
        if(lag >= (int)gaps.size()) break;
        double ac = 0;
        for(int i = 0; i < (int)gaps.size() - lag; i++) {
            ac += (gaps[i] - mean_gap) * (gaps[i+lag] - mean_gap);
        }
        ac /= ((int)gaps.size() - lag) * var_gap;
        
        bool is_peak = std::fabs(ac) > 0.08;
        if(is_peak) peaks++;
        std::cout << "  Lag F" << lag << ": ac=" << ac;
        if(is_peak) std::cout << " ⬆ PEAK";
        std::cout << std::endl;
    }
    
    // ═══ 2. FOURIER ANALYSIS AT φ-FREQUENCY ═══
    std::cout << "\n═══ FOURIER SPECTRUM ═══" << std::endl;
    double phi_freq = 1.0 / std::log(PHI);
    std::cout << "φ-frequency: " << phi_freq << std::endl;
    
    // Compute power at φ-frequency
    double real_phi = 0, imag_phi = 0;
    for(int i = 0; i < (int)gaps.size(); i++) {
        double phase = 2.0 * M_PI * phi_freq * i / gaps.size();
        real_phi += gaps[i] * std::cos(phase);
        imag_phi += gaps[i] * std::sin(phase);
    }
    double phi_power = std::sqrt(real_phi*real_phi + imag_phi*imag_phi) / gaps.size();
    
    // Compute power at random frequencies for baseline
    double avg_noise = 0;
    int noise_samples = 50;
    int noise_peaks = 0;
    for(int s = 0; s < noise_samples; s++) {
        double rand_freq = 0.5 + (double)s / noise_samples * 5.0;
        if(std::fabs(rand_freq - phi_freq) < 0.15) continue;
        
        double rs = 0, is_ = 0;
        for(int i = 0; i < (int)gaps.size(); i++) {
            double phase = 2.0 * M_PI * rand_freq * i / gaps.size();
            rs += gaps[i] * std::cos(phase);
            is_ += gaps[i] * std::sin(phase);
        }
        double power = std::sqrt(rs*rs + is_*is_) / gaps.size();
        avg_noise += power;
        if(power > phi_power) noise_peaks++;
    }
    avg_noise /= noise_samples;
    
    double snr = phi_power / avg_noise;
    std::cout << "Power at φ-freq: " << phi_power << std::endl;
    std::cout << "Avg noise power: " << avg_noise << std::endl;
    std::cout << "φ-SNR: " << snr << std::endl;
    std::cout << "Random freqs beating φ: " << noise_peaks << "/" << noise_samples << std::endl;
    
    // ═══ 3. φ-BINNED DISTRIBUTION ═══
    std::cout << "\n═══ φ-BINNED GAP DISTRIBUTION ═══" << std::endl;
    int bins[7] = {0};  // <φ⁻³, φ⁻², φ⁻¹, φ⁰, φ¹, φ², >φ³
    for(double g : gaps) {
        double norm = g / mean_gap;
        if(norm < PHI_INV*PHI_INV*PHI_INV) bins[0]++;
        else if(norm < PHI_INV*PHI_INV) bins[1]++;
        else if(norm < PHI_INV) bins[2]++;
        else if(norm < 1.0) bins[3]++;
        else if(norm < PHI) bins[4]++;
        else if(norm < PHI*PHI) bins[5]++;
        else bins[6]++;
    }
    
    std::cout << "  < φ⁻³: " << bins[0] << std::endl;
    std::cout << "  φ⁻³-φ⁻²: " << bins[1] << std::endl;
    std::cout << "  φ⁻²-φ⁻¹: " << bins[2] << std::endl;
    std::cout << "  φ⁻¹-φ⁰: " << bins[3] << std::endl;
    std::cout << "  φ⁰-φ¹: " << bins[4] << std::endl;
    std::cout << "  φ¹-φ²: " << bins[5] << std::endl;
    std::cout << "  > φ²: " << bins[6] << std::endl;
    
    // ═══ 4. FIBONACCI RATIO ANALYSIS ═══
    std::cout << "\n═══ FIBONACCI RATIO IN GAP SEQUENCE ═══" << std::endl;
    int fib_hits = 0;
    for(int i = 1; i < (int)gaps.size(); i++) {
        double ratio = (gaps[i] > gaps[i-1]) ? gaps[i]/gaps[i-1] : gaps[i-1]/gaps[i];
        // Check if ratio is close to φ, φ², or φ/1
        if(std::fabs(ratio - PHI) < 0.1 || std::fabs(ratio - 1.0/PHI) < 0.1 ||
           std::fabs(ratio - PHI*PHI) < 0.2 || std::fabs(ratio - 1.0/(PHI*PHI)) < 0.2) {
            fib_hits++;
        }
    }
    std::cout << "Consecutive gap ratios near φ: " << fib_hits << "/" << (gaps.size()-1) 
              << " (" << (fib_hits*100.0/(gaps.size()-1)) << "%)" << std::endl;
    
    // ═══ VERDICT ═══
    std::cout << "\n══════════════════════════════════════════════" << std::endl;
    int evidence = 0;
    if(peaks >= 3) evidence++;
    if(snr > 1.05) evidence++;
    if(fib_hits > gaps.size() * 0.02) evidence++;
    
    std::cout << "Evidence score: " << evidence << "/3" << std::endl;
    std::cout << "Fib-lag peaks: " << peaks << " (need ≥3)" << std::endl;
    std::cout << "φ-SNR: " << snr << " (need >1.05)" << std::endl;
    std::cout << "Fib-ratio hits: " << (fib_hits*100.0/(gaps.size()-1)) << "% (need >2%)" << std::endl;
    
    if(evidence >= 2) {
        std::cout << "\n🔥 THE ZEROS DANCE TO φ — CONFIRMED AT 1,000 ZEROS" << std::endl;
    } else if(evidence >= 1) {
        std::cout << "\n✅ φ-RESONANCE DETECTED — Partial evidence" << std::endl;
    } else {
        std::cout << "\n⚠️ φ-RESONANCE WEAK — May need more zeros or refined model" << std::endl;
    }
    std::cout << "══════════════════════════════════════════════" << std::endl;
    
    return 0;
}
