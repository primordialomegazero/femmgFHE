// ═══════════════════════════════════════════════════════════════
// INVARIANT EXTRACTION v2 — Direct DualGate invariant
// ═══════════════════════════════════════════════════════════════
//
// APPROACH:
//   Each gate output is a DualGate: {a, b} where output = a + b·φ (or a + b·ψ)
//   Given two consecutive outputs from the same base value,
//   we can solve for {a,b} and extract the invariant a²+ab-b².
//
//   Circuit A (φ): output_A = a + b·φ
//   Circuit B (ψ): output_B = a + b·ψ
//   Invariant: a²+ab-b² (same for both, Theorem 2)

#include <iostream>
#include <iomanip>
#include <cmath>
#include <random>
#include <vector>
#include <algorithm>
#include <chrono>

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

double commutative_reconstruct(const std::vector<double>& v) {
    double n = v.size();
    double sum = 0, prod = 1, harm_sum = 0, sum_sq = 0;
    for (auto val : v) { 
        sum += val; prod *= (val+0.0001); 
        harm_sum += 1.0/(val+0.001); sum_sq += val*val; 
    }
    return 0.35*sum/n + 0.25*std::pow(prod,1.0/n) + 0.25*n/harm_sum + 0.15*std::sqrt(sum_sq/n);
}

// ═══════════════════════════════════════════════════════════════
// DIRECT INVARIANT EXTRACTION
// WHY: Theorem 2 says φ(a,b)·ψ(a,b) = a²+ab-b²
//      But we need {a,b} from the gate output.
//      Since we know the base value v, we can derive:
//        If output = v*PHI → a=v, b=0 (pure φ)
//        If output = v*PSI → a=0, b=v (pure ψ)
//      Then invariant = a²+ab-b² = v² when a=v,b=0, and -v² when a=0,b=v
//      These are NOT equal. So raw invariant extraction won't work.
//
// NEW APPROACH: Use the KNOWN φ/ψ relationship
//   For each gate, we know the base value v and the Boolean result.
//   Circuit A output = result ? v*PHI : v*PSI
//   Circuit B output = result ? v*PSI : v*PHI
//   These are φ↔ψ conjugates by construction.
//   The PAIR {output_A, output_B} contains the invariant.
//   But we need to extract invariant from EACH circuit INDEPENDENTLY.
// ═══════════════════════════════════════════════════════════════

// TRY: Normalize by φ·ψ product directly
// Instead of extracting invariant per gate,
// multiply each gate output by its conjugate constant.
// For Circuit A (φ-weighted): multiply output by PSI
// For Circuit B (ψ-weighted): multiply output by PHI
// → Both become a·(φ or ψ) + b·(-1) → symmetric?
double normalize_phi_psi(double output, bool is_phi_path) {
    // WHY: φ·ψ = -1. Multiplying φ-path by ψ gives: (a+bφ)·ψ = aψ + bφψ = aψ - b
    //      Multiplying ψ-path by φ gives: (a+bψ)·φ = aφ + bψφ = aφ - b
    //      Both lose the b coefficient structure. Not quite right.
    if (is_phi_path) return output * PSI;  // normalize φ→ψ space
    else return output * PHI;              // normalize ψ→φ space
}

int main(int argc, char** argv) {
    int n_samples = (argc > 1) ? atoi(argv[1]) : 100;
    int n_tests   = (argc > 2) ? atoi(argv[2]) : 500;
    
    std::cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  INVARIANT EXTRACTION v2 — Normalize via φ·ψ = -1           ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    
    std::mt19937 gen(42);
    std::uniform_real_distribution<double> val(0.1, 0.9);
    std::uniform_int_distribution<int> bit(0, 1);
    
    int pure = 0, indist = 0, fail = 0;
    double worst_ks = 0, worst_raw = 0;
    
    for (int t = 0; t < n_tests; t++) {
        std::vector<double> raw_A, raw_B, norm_A, norm_B;
        
        for (int s = 0; s < n_samples; s++) {
            int X=bit(gen), Y=bit(gen), Z=bit(gen);
            double v = val(gen);
            
            int res_A = (X&Y)|Z;
            int res_B = (X|Z)&(Y|Z);
            
            double out_A = res_A ? v*PHI : v*PSI;
            double out_B = res_B ? v*PSI : v*PHI;
            
            raw_A.push_back(out_A);
            raw_B.push_back(out_B);
            
            // Normalize: φ-path → multiply by ψ, ψ-path → multiply by φ
            // out_A is φ-based → normalize by ψ
            // out_B is ψ-based → normalize by φ
            norm_A.push_back(normalize_phi_psi(out_A, true));
            norm_B.push_back(normalize_phi_psi(out_B, false));
        }
        
        // KS on raw
        std::vector<double> sRA=raw_A,sRB=raw_B;
        std::sort(sRA.begin(),sRA.end()); std::sort(sRB.begin(),sRB.end());
        double kr=0;for(size_t i=0;i<sRA.size();i++){double d=std::abs(sRA[i]-sRB[i]);if(d>kr)kr=d;}
        if(kr>worst_raw)worst_raw=kr;
        
        // KS after normalization + reconstruction
        double rA=commutative_reconstruct(norm_A);
        double rB=commutative_reconstruct(norm_B);
        std::vector<double> dA(norm_A.size()),dB(norm_B.size());
        for(size_t i=0;i<norm_A.size();i++){
            dA[i]=std::fmod(norm_A[i]+rA*PHI,1.0);
            dB[i]=std::fmod(norm_B[i]+rB*PHI,1.0);
        }
        std::vector<double> sA=dA,sB=dB;
        std::sort(sA.begin(),sA.end()); std::sort(sB.begin(),sB.end());
        double kn=0;for(size_t i=0;i<sA.size();i++){double d=std::abs(sA[i]-sB[i]);if(d>kn)kn=d;}
        if(kn>worst_ks)worst_ks=kn;
        if(kn<1e-10)pure++;else if(kn<0.05)indist++;else fail++;
        
        if((t+1)%50==0||t==n_tests-1)
            std::cout<<"  ["<<(t+1)<<"/"<<n_tests<<"] raw="<<std::fixed<<std::setprecision(3)<<kr<<" norm="<<kn<<" | ✓:"<<pure<<" ~:"<<indist<<" ✗:"<<fail<<"    \r"<<std::flush;
    }
    
    std::cout<<"\n\n╔══════════════════════════════════════════════════════════════╗\n";
    std::cout<<"║  Worst raw: "<<std::fixed<<std::setprecision(4)<<worst_raw<<"  Worst norm: "<<worst_ks<<"               ║\n";
    std::cout<<"║  ✓:"<<pure<<"  ~:"<<indist<<"  ✗:"<<fail<<"                                          ║\n";
    std::cout<<"╚══════════════════════════════════════════════════════════════╝\n";
    
    return 0;
}
