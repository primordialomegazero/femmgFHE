#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>
#include <iomanip>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    long Q = GoldenFHE::Q;
    long golden_plain = static_cast<long>(Q / GoldenFHE::PHI);
    long threshold = static_cast<long>(Q / (2 * GoldenFHE::PHI));
    
    std::cout << "Q: " << Q << "\n";
    std::cout << "golden_plain: " << golden_plain << "\n";
    std::cout << "threshold: " << threshold << "\n\n";
    
    // I-check ang scaling
    for (long new_q : {Q, Q/2, Q/4, Q/8}) {
        // Scale ang golden_plain
        long scaled_golden = (golden_plain * new_q + Q/2) / Q;
        long scaled_threshold = (threshold * new_q + Q/2) / Q;
        
        std::cout << "new_q: " << new_q << "\n";
        std::cout << "  scaled_golden: " << scaled_golden << "\n";
        std::cout << "  scaled_threshold: " << scaled_threshold << "\n";
        std::cout << "  golden > threshold? " << (scaled_golden > scaled_threshold ? "YES" : "NO") << "\n\n";
    }
    
    // Ang problema: kapag nag-scale tayo, ang golden_plain ay nagiging mas maliit
    // sa threshold, kaya na-mi-misclassify bilang 0
    
    // Solusyon: Dapat i-scale nang tama ang threshold
    // Para sa modulus switching, ang decryption threshold ay dapat din ma-scale
    
    return 0;
}
