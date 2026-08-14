#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>
#include <random>
#include <iomanip>

class NoiseFlooding {
private:
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    std::mt19937 rng;
    
public:
    NoiseFlooding(const GoldenFHE::PublicKey& public_key,
                  const GoldenFHE::SecretKey& secret_key,
                  int seed = 42)
        : pk(public_key), sk(secret_key), rng(seed) {}
    
    GoldenFHE::Cipher flood_noise(const GoldenFHE::Cipher& ct, double noise_level) {
        GoldenFHE::Cipher result = ct;
        std::uniform_int_distribution<long> noise_dist(0, static_cast<long>(noise_level));
        long noise = noise_dist(rng);
        NTL::ZZ_pX noise_poly;
        NTL::SetCoeff(noise_poly, 0, noise);
        result.c0 = result.c0 + noise_poly;
        return result;
    }
    
    bool test_noise_flooding(const GoldenFHE::Cipher& original, double noise_level) {
        GoldenFHE::Cipher flooded = flood_noise(original, noise_level);
        bool before = GoldenFHE::decrypt(original, sk);
        bool after = GoldenFHE::decrypt(flooded, sk);
        return before == after;
    }
};

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    std::cout << "Testing Noise Flooding...\n\n";
    
    NoiseFlooding nf(pk, sk);
    
    long golden_plain = static_cast<long>(GoldenFHE::Q / GoldenFHE::PHI);
    long threshold = static_cast<long>(GoldenFHE::Q / (2 * GoldenFHE::PHI));
    long noise_budget = threshold - 1;
    
    std::cout << "Golden plain: " << golden_plain << "\n";
    std::cout << "Threshold: " << threshold << "\n";
    std::cout << "Max noise budget: " << noise_budget << "\n\n";
    
    std::vector<double> noise_levels = {
        1000, 10000, 100000, 1000000, 10000000, 50000000, 100000000
    };
    
    GoldenFHE::Cipher ct = GoldenFHE::encrypt(pk, true, 1000000);
    
    std::cout << "Noise Level    Decrypt    Status\n";
    std::cout << "-----------    -------    ------\n";
    
    for (double noise_level : noise_levels) {
        bool preserved = nf.test_noise_flooding(ct, noise_level);
        bool decrypted = GoldenFHE::decrypt(nf.flood_noise(ct, noise_level), sk);
        
        std::cout << std::setw(11) << static_cast<long>(noise_level) << "    "
                  << std::setw(7) << decrypted << "    "
                  << (preserved ? "OK" : "FAIL") << "\n";
    }
    
    std::cout << "\n✅ NOISE FLOODING TEST COMPLETE!\n";
    return 0;
}
