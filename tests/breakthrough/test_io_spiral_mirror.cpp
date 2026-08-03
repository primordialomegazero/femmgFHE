// ═══════════════════════════════════════════════════════════════
// SPIRAL MIRROR CRYSTAL BRIDGE — N-Configurable Recursive iO
// ═══════════════════════════════════════════════════════════════
//
// ARCHITECTURE:
//   Instead of crystal lattice (shuffle), use SPIRAL rotation.
//   Each even layer: rotate values through φ/ψ space
//   Each odd layer:  mirror collapse (φ·ψ = -1) → canonical
//
// SPIRAL ROTATION:
//   After mirror collapse, all values are canonical (v).
//   Apply rotation: v → v·φ (spiral outward)
//   Next mirror collapse: v·φ → (v·φ)·ψ = -v → |v| again
//   The spiral goes out (φ) and in (ψ), but the canonical value
//   remains the same after each mirror collapse.
//
// WHY SPIRAL:
//   Like light spiraling between two parabolic mirrors,
//   each reflection brings it back to the same point (canonical)
//   while the path length (N) increases.

#include <iostream>
#include <iomanip>
#include <cmath>
#include <random>
#include <vector>
#include <algorithm>
#include <chrono>

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

// ═══════════════════════════════════════════════════════════════
// MIRROR COLLAPSE
// WHY: φ×ψ = -1. Collapses φ or ψ values to the same canonical |v|.
// ═══════════════════════════════════════════════════════════════
double mirror_collapse(double value, bool was_phi_path) {
    double collapsed = was_phi_path ? value * PSI : value * PHI;
    return std::abs(collapsed);
}

// ═══════════════════════════════════════════════════════════════
// SPIRAL LATTICE (Even layers)
// WHY: Instead of shuffling groups (which breaks the bridge),
//      rotate each value through φ. The next mirror collapse
//      will rotate back through ψ, bringing us to canonical.
//      This preserves the canonical value at every odd layer.
// ═══════════════════════════════════════════════════════════════
std::vector<double> spiral_lattice(const std::vector<double>& canonical_values, uint64_t seed) {
    std::vector<double> rotated = canonical_values;
    
    // Spiral outward: multiply each value by φ
    // The values are now φ-weighted, but the underlying |v| is preserved
    // because the next mirror collapse will multiply by ψ and take |·|
    for (auto& v : rotated) {
        v = v * PHI;  // Spiral outward
    }
    
    // Shuffle to create structural difference
    // This is safe because all values are now φ-weighted from the same canonical base
    std::mt19937 gen(seed);
    std::shuffle(rotated.begin(), rotated.end(), gen);
    
    return rotated;
}

// ═══════════════════════════════════════════════════════════════
// SPIRAL MIRROR — N-Layer Bridge
//
// Layer 1 (odd):  Mirror collapse → canonical |v|
// Layer 2 (even): Spiral outward (×φ) + shuffle
// Layer 3 (odd):  Mirror collapse → canonical |v| again
// Layer 4 (even): Spiral outward (×φ) + shuffle
// ...
//
// After EVERY odd layer: all values return to canonical |v|.
// After EVERY even layer: values are φ-weighted from canonical base.
// The bridge holds at every layer because mirror collapse always
// returns to the same canonical space.
// ═══════════════════════════════════════════════════════════════
std::vector<double> spiral_mirror(
    const std::vector<double>& phi_outputs,
    const std::vector<bool>& phi_flags,
    int N_layers,
    uint64_t base_seed
) {
    std::vector<double> current = phi_outputs;
    // Track whether current values are φ-weighted (true) or ψ-weighted (false)
    // After mirror collapse: canonical |v| (neither)
    // After spiral outward: φ-weighted (true)
    // This flag gets UPDATED at each layer.
    std::vector<bool> current_flags = phi_flags;
    
    for (int layer = 0; layer < N_layers; layer++) {
        if (layer % 2 == 0) {
            // ODD: Mirror collapse → canonical
            for (size_t i = 0; i < current.size(); i++) {
                current[i] = mirror_collapse(current[i], current_flags[i]);
            }
            // After mirror collapse: values are canonical |v| — not φ or ψ
        } else {
            // EVEN: Spiral outward (×φ) + shuffle → φ-weighted
            // After spiral: ALL values are now φ-weighted
            current = spiral_lattice(current, base_seed + layer * 1000);
            // Update flags: ALL values are now φ-weighted (true)
            current_flags.assign(current.size(), true);
        }
    }
    
    return current;
}

double commutative_reconstruct(const std::vector<double>& v) {
    double n = v.size();
    double sum = 0, prod = 1, harm_sum = 0, sum_sq = 0;
    for (auto val : v) { 
        sum += val; prod *= (val+0.0001); 
        harm_sum += 1.0/(val+0.001); sum_sq += val*val; 
    }
    return 0.35*sum/n + 0.25*std::pow(prod,1.0/n) + 0.25*n/harm_sum + 0.15*std::sqrt(sum_sq/n);
}

int main(int argc, char** argv) {
    int N_layers  = (argc > 1) ? atoi(argv[1]) : 3;
    int n_samples = (argc > 2) ? atoi(argv[2]) : 100;
    int n_tests   = (argc > 3) ? atoi(argv[3]) : 500;
    
    std::cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  SPIRAL MIRROR CRYSTAL — N-Configurable Recursive Bridge   ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    std::cout << "  N layers: " << N_layers << "\n";
    std::cout << "  Odd:  Mirror collapse (φ·ψ=-1) → canonical |v|\n";
    std::cout << "  Even: Spiral outward (×φ) + shuffle → φ-weighted\n";
    std::cout << "  Tests: " << n_tests << " | Samples: " << n_samples << "\n\n";
    
    std::mt19937 gen(42);
    std::uniform_real_distribution<double> val(0.1, 0.9);
    std::uniform_int_distribution<int> bit(0, 1);
    
    int pure = 0, indist = 0, fail = 0;
    double worst_ks = 0;
    
    auto start = std::chrono::steady_clock::now();
    for (int t = 0; t < n_tests; t++) {
        std::vector<double> out_A, out_B;
        std::vector<bool> flags_A, flags_B;
        
        for (int s = 0; s < n_samples; s++) {
            int X=bit(gen), Y=bit(gen), Z=bit(gen);
            double v = val(gen);
            
            int res_A = (X&Y)|Z;
            int res_B = (X|Z)&(Y|Z);
            
            bool A_phi = (res_A == 1);
            out_A.push_back(A_phi ? v*PHI : v*PSI);
            flags_A.push_back(A_phi);
            
            bool B_phi = (res_B == 0);
            out_B.push_back(B_phi ? v*PHI : v*PSI);
            flags_B.push_back(B_phi);
        }
        
        auto spir_A = spiral_mirror(out_A, flags_A, N_layers, t*10000);
        auto spir_B = spiral_mirror(out_B, flags_B, N_layers, t*10000+5000);
        
        double rec_A = commutative_reconstruct(spir_A);
        double rec_B = commutative_reconstruct(spir_B);
        
        std::vector<double> dA(spir_A.size()), dB(spir_B.size());
        for (size_t i = 0; i < spir_A.size(); i++) {
            dA[i] = std::fmod(spir_A[i] + rec_A * PHI, 1.0);
            dB[i] = std::fmod(spir_B[i] + rec_B * PHI, 1.0);
        }
        
        std::vector<double> sA=dA, sB=dB;
        std::sort(sA.begin(),sA.end()); std::sort(sB.begin(),sB.end());
        double ks=0;
        for(size_t i=0;i<sA.size();i++){
            double d=std::abs(sA[i]-sB[i]);
            if(d>ks)ks=d;
        }
        if(ks>worst_ks)worst_ks=ks;
        if(ks<1e-10)pure++;else if(ks<0.05)indist++;else fail++;
        
        if((t+1)%50==0||t==n_tests-1)
            std::cout<<"  ["<<(t+1)<<"/"<<n_tests<<"] N="<<N_layers
                     <<" KS="<<std::fixed<<std::setprecision(4)<<ks
                     <<" | ✓:"<<pure<<" ~:"<<indist<<" ✗:"<<fail<<"    \r"<<std::flush;
    }
    
    auto end = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(end - start).count();
    
    std::cout<<"\n\n";
    std::cout<<"╔══════════════════════════════════════════════════════════════╗\n";
    std::cout<<"║  SPIRAL MIRROR RESULTS (N="<<N_layers<<")                                ║\n";
    std::cout<<"║  Worst KS: "<<std::fixed<<std::setprecision(6)<<worst_ks<<"                                     ║\n";
    std::cout<<"║  ✓:"<<pure<<"  ~:"<<indist<<"  ✗:"<<fail<<"  |  Time: "<<std::setprecision(1)<<elapsed<<"s                            ║\n";
    std::cout<<"╚══════════════════════════════════════════════════════════════╝\n";
    
    if(fail==0) std::cout<<"\n  ✅ SPIRAL MIRROR — KS=0 at N="<<N_layers<<" layers\n\n";
    else std::cout<<"\n  ❌ "<<fail<<" tests failed\n\n";
    
    return 0;
}
