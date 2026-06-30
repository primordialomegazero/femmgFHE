#include "src/riemann_zeros_200.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <algorithm>
#include <numeric>

constexpr double PHI = 1.6180339887498948482;
constexpr double PHI_INV = 0.6180339887498948482;

using namespace femmg::riemann;

int main() {
    std::cout << "╔══════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  SOURCE FREQUENCY: Is it φ⁻¹ ≈ 0.618?        ║" << std::endl;
    std::cout << "║  'Golden ratio is the weakness of infinity'  ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════╝" << std::endl;
    
    int N = ZEROS_200_COUNT;
    std::vector<double> gaps;
    for(int i = 1; i < N; i++) {
        gaps.push_back(ZEROS_200[i] - ZEROS_200[i-1]);
    }
    double mean_gap = std::accumulate(gaps.begin(), gaps.end(), 0.0) / gaps.size();
    
    // ═══ FULL FREQUENCY SWEEP (High Resolution) ═══
    std::cout << "\n═══ FREQUENCY SWEEP (0.1 to 5.0, step 0.001) ═══" << std::endl;
    std::cout << "Scanning " << gaps.size() << " gaps for dominant frequencies..." << std::endl;
    
    std::vector<std::pair<double,double>> spectrum;  // (frequency, power)
    double max_power = 0, max_freq = 0;
    double phi_power = 0, phi_inv_power = 0, phi2_power = 0;
    
    double phi_freq = 1.0 / std::log(PHI);  // 2.078
    double phi_inv_freq = PHI_INV;           // 0.618
    double phi2_freq = PHI * PHI;            // 2.618
    
    for(double f = 0.1; f <= 5.0; f += 0.001) {
        double rs = 0, is = 0;
        for(int i = 0; i < (int)gaps.size(); i++) {
            double phase = 2.0 * M_PI * f * i / gaps.size();
            double normalized = gaps[i] - mean_gap;
            rs += normalized * std::cos(phase);
            is += normalized * std::sin(phase);
        }
        double power = std::sqrt(rs*rs + is*is) / gaps.size();
        spectrum.push_back({f, power});
        
        if(power > max_power) {
            max_power = power;
            max_freq = f;
        }
        
        if(std::fabs(f - phi_freq) < 0.001) phi_power = power;
        if(std::fabs(f - phi_inv_freq) < 0.001) phi_inv_power = power;
        if(std::fabs(f - phi2_freq) < 0.001) phi2_power = power;
    }
    
    std::cout << "\n═══ RESULTS ═══" << std::endl;
    std::cout << "Dominant frequency: f = " << max_freq << " (power = " << max_power << ")" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "φ-frequencies:" << std::endl;
    std::cout << "  φ⁻¹ = " << PHI_INV << " → power = " << phi_inv_power 
              << " (" << (phi_inv_power/max_power*100.0) << "% of max)" << std::endl;
    std::cout << "  1/ln(φ) = " << phi_freq << " → power = " << phi_power 
              << " (" << (phi_power/max_power*100.0) << "% of max)" << std::endl;
    std::cout << "  φ² = " << phi2_freq << " → power = " << phi2_power 
              << " (" << (phi2_power/max_power*100.0) << "% of max)" << std::endl;
    
    // ═══ FIND NEAREST PEAK TO φ⁻¹ ═══
    std::cout << "\n═══ PEAKS NEAR φ-VALUES ═══" << std::endl;
    
    auto find_nearest_peak = [&](double target) {
        double best_power = 0, best_freq = target;
        for(int i = 1; i < (int)spectrum.size() - 1; i++) {
            // Local maximum
            if(spectrum[i].second > spectrum[i-1].second && 
               spectrum[i].second > spectrum[i+1].second) {
                if(std::fabs(spectrum[i].first - target) < 0.1) {
                    if(spectrum[i].second > best_power) {
                        best_power = spectrum[i].second;
                        best_freq = spectrum[i].first;
                    }
                }
            }
        }
        return std::make_pair(best_freq, best_power);
    };
    
    auto peak_phi_inv = find_nearest_peak(PHI_INV);
    auto peak_phi = find_nearest_peak(PHI);
    auto peak_phi2 = find_nearest_peak(PHI*PHI);
    auto peak_phi_freq = find_nearest_peak(phi_freq);
    
    std::cout << "Nearest peak to φ⁻¹=" << PHI_INV << ": f=" << peak_phi_inv.first 
              << " power=" << peak_phi_inv.second << std::endl;
    std::cout << "Nearest peak to φ=" << PHI << ": f=" << peak_phi.first 
              << " power=" << peak_phi.second << std::endl;
    std::cout << "Nearest peak to φ²=" << PHI*PHI << ": f=" << peak_phi2.first 
              << " power=" << peak_phi2.second << std::endl;
    
    // ═══ TOP 10 FREQUENCIES ═══
    std::cout << "\n═══ TOP 10 FREQUENCIES ═══" << std::endl;
    std::sort(spectrum.begin(), spectrum.end(), 
              [](auto& a, auto& b) { return a.second > b.second; });
    
    for(int i = 0; i < 10; i++) {
        double f = spectrum[i].first;
        double p = spectrum[i].second;
        
        // Check proximity to φ-related values
        std::string note = "";
        if(std::fabs(f - PHI_INV) < 0.02) note = " ← φ⁻¹ (SOURCE FREQUENCY)";
        else if(std::fabs(f - PHI) < 0.02) note = " ← φ";
        else if(std::fabs(f - phi_freq) < 0.02) note = " ← 1/ln(φ)";
        else if(std::fabs(f - (PHI*PHI)) < 0.02) note = " ← φ²";
        else if(std::fabs(f - 1.0) < 0.02) note = " ← unity";
        
        std::cout << "  " << (i+1) << ". f=" << f << " power=" << p << note << std::endl;
    }
    
    // ═══ VERDICT ═══
    std::cout << "\n══════════════════════════════════════════════" << std::endl;
    
    double phi_inv_ratio = phi_inv_power / max_power;
    double phi_ratio = phi_power / max_power;
    
    if(std::fabs(max_freq - PHI_INV) < 0.05) {
        std::cout << "🔥 THE DOMINANT FREQUENCY IS φ⁻¹ ≈ 0.618" << std::endl;
        std::cout << "   'Golden ratio is the weakness of infinity'" << std::endl;
        std::cout << "   The Source hums at φ⁻¹. The zeros dance to it." << std::endl;
    } else if(phi_inv_ratio > 0.70) {
        std::cout << "✅ φ⁻¹ IS A MAJOR HARMONIC (" << (phi_inv_ratio*100.0) << "% of max)" << std::endl;
        std::cout << "   The Source frequency is strongly present." << std::endl;
    } else if(phi_inv_ratio > 0.40) {
        std::cout << "⚠️ φ⁻¹ IS A SECONDARY HARMONIC (" << (phi_inv_ratio*100.0) << "% of max)" << std::endl;
        std::cout << "   Present but not dominant in this sample." << std::endl;
    } else {
        std::cout << "❌ φ⁻¹ NOT DETECTED AS SIGNIFICANT" << std::endl;
    }
    std::cout << "══════════════════════════════════════════════" << std::endl;
    
    return 0;
}
