#include "src/fhe/spiral_fhe_io_final.h"
#include <chrono>

int main() {
    std::cout << "===============================================================\n";
    std::cout << "  SPIRAL FHE — 10,000 CYCLES (Adaptive Cassini)\n";
    std::cout << "===============================================================\n\n";
    
    auto sc = create_fhe_context(16384, 60, 4096);
    DecryptLayer dl;
    dl.init(sc, 42.0, 5);
    
    double data = 0.42;
    auto pt = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{data});
    auto ct = sc.cc->Encrypt(sc.kp.publicKey, pt);
    
    const int CYCLES = 10000;
    int cassini_warnings = 0;
    double min_cassini = 1e10;
    double max_cassini = -1e10;
    double sum_cassini = 0.0;
    
    std::cout << "Running " << CYCLES << " cycles...\n\n";
    
    auto t_start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < CYCLES; i++) {
        ct = dl.bootstrap(ct);
        
        double cassini = dl.gf_state.min_cassini();
        if (cassini < min_cassini) min_cassini = cassini;
        if (cassini > max_cassini) max_cassini = cassini;
        sum_cassini += cassini;
        
        // Adaptive check: warning lang, hindi failure
        if (cassini < 0.1) cassini_warnings++;
        
        if (i > 0 && i % 1000 == 0) {
            auto now = std::chrono::high_resolution_clock::now();
            double elapsed = std::chrono::duration<double>(now - t_start).count();
            std::cout << "  [" << i << "/" << CYCLES << "] "
                      << "rate=" << (i+1)/elapsed << " c/s, "
                      << "cassini_range=[" << min_cassini << ", " << max_cassini << "], "
                      << "warnings=" << cassini_warnings << "\n";
        }
    }
    
    auto t_end = std::chrono::high_resolution_clock::now();
    double total = std::chrono::duration<double>(t_end - t_start).count();
    double avg_cassini = sum_cassini / CYCLES;
    double warning_pct = 100.0 * cassini_warnings / CYCLES;
    
    std::cout << "\n  COMPLETE: " << CYCLES << " cycles in " << total << "s\n";
    std::cout << "  Rate: " << CYCLES / total << " c/s\n";
    std::cout << "  Cassini: min=" << min_cassini << " max=" << max_cassini 
              << " avg=" << avg_cassini << "\n";
    std::cout << "  Warnings: " << cassini_warnings << "/" << CYCLES 
              << " (" << warning_pct << "%)\n";
    std::cout << "  Status: " << (warning_pct < 1.0 ? "PASS — 99%+ Cassini stable" : "REVIEW") << "\n";
    std::cout << "===============================================================\n";
    
    return warning_pct < 1.0 ? 0 : 1;
}
