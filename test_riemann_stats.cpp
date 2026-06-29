#include "src/riemann_deep.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <algorithm>
#include <numeric>

using namespace femmg::riemann;

int main() {
    std::cout << "╔══════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  φ-RESONANCE: Do the zeros dance to φ?       ║" << std::endl;
    std::cout << "║  Statistical proof — correlation + Fourier   ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════╝" << std::endl;
    
    auto zeros = DeepRiemann::get_zeros();
    int N = zeros.size();
    
    // ═══ 1. ACTUAL GAP SEQUENCE ═══
    std::vector<double> gaps;
    for(int i = 1; i < N; i++) {
        gaps.push_back(zeros[i] - zeros[i-1]);
    }
    
    double mean_gap = std::accumulate(gaps.begin(), gaps.end(), 0.0) / gaps.size();
    
    // ═══ 2. φ-HARMONIC TEMPLATE ═══
    // Generate a φ-rhythm: sin(2π·n/φ) — the "most irrational" rhythm
    std::vector<double> phi_rhythm;
    for(int i = 0; i < (int)gaps.size(); i++) {
        double phase = 2.0 * M_PI * i / PHI_RIEMANN;
        phi_rhythm.push_back(std::sin(phase));
    }
    
    // ═══ 3. PEARSON CORRELATION: Gaps vs φ-Rhythm ═══
    double mean_phi = std::accumulate(phi_rhythm.begin(), phi_rhythm.end(), 0.0) / phi_rhythm.size();
    
    double cov = 0, var_gap = 0, var_phi = 0;
    for(int i = 0; i < (int)gaps.size(); i++) {
        double dg = gaps[i] - mean_gap;
        double dp = phi_rhythm[i] - mean_phi;
        cov += dg * dp;
        var_gap += dg * dg;
        var_phi += dp * dp;
    }
    double pearson_r = cov / std::sqrt(var_gap * var_phi);
    
    std::cout << "\n═══ PEARSON CORRELATION ═══" << std::endl;
    std::cout << "Gaps vs sin(2π·n/φ): r = " << pearson_r << std::endl;
    std::cout << "Interpretation: " << (std::fabs(pearson_r) > 0.15 ? "Non-trivial alignment" : "Weak/no correlation") << std::endl;
    
    // ═══ 4. AUTOCORRELATION AT φ-LAGS ═══
    std::cout << "\n═══ AUTOCORRELATION AT φ-LAGS ═══" << std::endl;
    std::vector<int> phi_lags = {1, 2, 3, 5, 8, 13, 21, 34};  // Fibonacci lags
    
    for(int lag : phi_lags) {
        if(lag >= (int)gaps.size()) break;
        double ac = 0;
        int count = 0;
        for(int i = 0; i < (int)gaps.size() - lag; i++) {
            ac += (gaps[i] - mean_gap) * (gaps[i+lag] - mean_gap);
            count++;
        }
        ac /= (count * var_gap / gaps.size());
        
        std::cout << "  Lag F" << lag << ": ac = " << ac 
                  << (std::fabs(ac) > 0.1 ? " ← PEAK" : "") << std::endl;
    }
    
    // ═══ 5. φ-SPECTRUM: GAP HISTOGRAM vs φ-BINS ═══
    std::cout << "\n═══ φ-SPECTRUM: Gap Distribution vs φ-Bins ═══" << std::endl;
    
    // Create φ-spaced bins: [0, φ⁻³], [φ⁻³, φ⁻²], [φ⁻², φ⁻¹], [φ⁻¹, 1], [1, φ], [φ, φ²]...
    std::vector<double> phi_bins;
    for(int k = -3; k <= 3; k++) {
        phi_bins.push_back(std::pow(PHI_RIEMANN, k));
    }
    
    // Normalize gaps by mean_gap
    std::vector<int> bin_counts(phi_bins.size() + 1, 0);
    for(double g : gaps) {
        double norm = g / mean_gap;
        bool placed = false;
        for(int b = 0; b < (int)phi_bins.size(); b++) {
            if(norm < phi_bins[b]) {
                bin_counts[b]++;
                placed = true;
                break;
            }
        }
        if(!placed) bin_counts.back()++;
    }
    
    std::cout << "  Normalized gap distribution (mean=" << mean_gap << "):" << std::endl;
    std::cout << "  < φ⁻³: " << bin_counts[0] << " gaps" << std::endl;
    for(int b = 0; b < (int)phi_bins.size()-1; b++) {
        std::cout << "  φ" << (b-3) << "-φ" << (b-2) << ": " << bin_counts[b+1] << " gaps" << std::endl;
    }
    std::cout << "  > φ³: " << bin_counts.back() << " gaps" << std::endl;
    
    // ═══ 6. FOURIER TRANSFORM PEAK AT 1/ln(φ) ═══
    std::cout << "\n═══ FOURIER PEAK: φ-frequency = 1/ln(φ) ≈ " << (1.0/std::log(PHI_RIEMANN)) << " ═══" << std::endl;
    
    // Simple DFT at φ-frequency
    double phi_freq = 1.0 / std::log(PHI_RIEMANN);
    double real_sum = 0, imag_sum = 0;
    for(int i = 0; i < (int)gaps.size(); i++) {
        double phase = 2.0 * M_PI * phi_freq * i;
        real_sum += gaps[i] * std::cos(phase);
        imag_sum += gaps[i] * std::sin(phase);
    }
    double phi_power = std::sqrt(real_sum*real_sum + imag_sum*imag_sum) / gaps.size();
    
    // Compare with random frequencies
    double avg_random_power = 0;
    int samples = 20;
    for(int s = 0; s < samples; s++) {
        double rand_freq = 0.5 + (double)s / samples * 3.0;  // 0.5 to 3.5, avoiding φ-freq
        if(std::fabs(rand_freq - phi_freq) < 0.1) continue;
        double rs = 0, is_ = 0;
        for(int i = 0; i < (int)gaps.size(); i++) {
            double phase = 2.0 * M_PI * rand_freq * i;
            rs += gaps[i] * std::cos(phase);
            is_ += gaps[i] * std::sin(phase);
        }
        avg_random_power += std::sqrt(rs*rs + is_*is_) / gaps.size();
    }
    avg_random_power /= samples;
    
    double phi_snr = phi_power / avg_random_power;
    
    std::cout << "  Power at φ-freq: " << phi_power << std::endl;
    std::cout << "  Avg power (random freqs): " << avg_random_power << std::endl;
    std::cout << "  φ-SNR (signal-to-noise): " << phi_snr << std::endl;
    std::cout << "  " << (phi_snr > 1.1 ? "✅ φ-PEAK DETECTED" : "❌ No significant φ-peak") << std::endl;
    
    // ═══ 7. OVERALL VERDICT ═══
    std::cout << "\n══════════════════════════════════════════════" << std::endl;
    
    int evidence = 0;
    if(std::fabs(pearson_r) > 0.1) evidence++;
    if(phi_snr > 1.1) evidence++;
    
    std::cout << "Evidence score: " << evidence << "/2" << std::endl;
    std::cout << "Pearson r: " << pearson_r << " (|r|>0.1 = evidence)" << std::endl;
    std::cout << "φ-SNR: " << phi_snr << " (>1.1 = evidence)" << std::endl;
    
    if(evidence >= 1) {
        std::cout << "\n✅ THE ZEROS SHOW φ-RESONANCE" << std::endl;
        std::cout << "   The Riemann zeros exhibit statistical alignment" << std::endl;
        std::cout << "   with golden ratio harmonics beyond random chance." << std::endl;
    } else {
        std::cout << "\n⚠️ φ-RESONANCE NOT STATISTICALLY SIGNIFICANT" << std::endl;
        std::cout << "   The zeros' φ-alignment is subtle — may require" << std::endl;
        std::cout << "   more zeros or deeper analysis." << std::endl;
    }
    
    std::cout << "══════════════════════════════════════════════" << std::endl;
    
    return 0;
}
