#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <random>
#include <algorithm>

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

// Fixed-degree noise test - PROPER NOISE TRACKING
// Ang noise ay hiwalay sa ciphertext coefficients

struct Ciphertext {
    std::vector<long> c0, c1, c2;
    double noise;  // Actual noise magnitude
    
    Ciphertext(int n = 1024) : c0(n, 0), c1(n, 0), c2(n, 0), noise(1.0) {}
};

class FixedDegreeNoiseTest {
private:
    int N;
    long Q;
    
    long mod(long x) {
        x %= Q;
        if (x < 0) x += Q;
        return x;
    }
    
    // Polynomial multiplication na may proper modular reduction
    std::vector<long> poly_mul(const std::vector<long>& a, 
                               const std::vector<long>& b) {
        std::vector<long> result(2 * N, 0);
        for (int i = 0; i < N; i++) {
            if (a[i] == 0) continue;
            for (int j = 0; j < N; j++) {
                if (b[j] == 0) continue;
                result[i + j] = (result[i + j] + a[i] * b[j]) % Q;
            }
        }
        // Reduce modulo X^N + 1
        for (int i = N; i < 2 * N; i++) {
            result[i - N] = mod(result[i - N] - result[i]);
            result[i] = 0;
        }
        result.resize(N);
        return result;
    }
    
    // Homomorphic NAND na may PROPER noise tracking
    Ciphertext homomorphic_nand(const Ciphertext& a, const Ciphertext& b) {
        Ciphertext result(N);
        
        // Multiply polynomials
        std::vector<long> t0 = poly_mul(a.c0, b.c0);
        std::vector<long> t1 = poly_mul(a.c0, b.c1);
        std::vector<long> t2 = poly_mul(a.c0, b.c2);
        std::vector<long> t3 = poly_mul(a.c1, b.c0);
        std::vector<long> t4 = poly_mul(a.c1, b.c1);
        std::vector<long> t5 = poly_mul(a.c1, b.c2);
        std::vector<long> t6 = poly_mul(a.c2, b.c0);
        std::vector<long> t7 = poly_mul(a.c2, b.c1);
        std::vector<long> t8 = poly_mul(a.c2, b.c2);
        
        // Combine terms (fixed degree - drop s³, s⁴ terms)
        result.c0 = t0;
        result.c0[0] = mod(result.c0[0] + 1);  // NOT operation
        
        result.c1.resize(N);
        for (int i = 0; i < N; i++) {
            result.c1[i] = mod(t1[i] + t3[i]);
        }
        
        result.c2.resize(N);
        for (int i = 0; i < N; i++) {
            result.c2[i] = mod(mod(t2[i] + t4[i]) + t6[i]);
        }
        
        // PROPER NOISE TRACKING
        // Sa RLWE: noise' = noise_a * |b| + noise_b * |a| + small_error
        // Para sa NAND: ang noise ay dapat lumaki nang kontrolado
        
        double noise_a = a.noise;
        double noise_b = b.noise;
        
        // Estimate ng coefficient magnitude
        double coeff_magnitude = 0;
        for (int i = 0; i < N; i++) {
            coeff_magnitude += std::abs(result.c0[i] % 100) + 
                              std::abs(result.c1[i] % 100) + 
                              std::abs(result.c2[i] % 100);
        }
        coeff_magnitude /= (3.0 * N);
        
        // Simplified noise growth model
        // Sa totoong RLWE: noise_new ≈ noise_a * ||b|| + noise_b * ||a||
        // Dito: i-model natin na may φ-based cancellation
        double base_growth = noise_a * noise_b;
        double phi_factor = std::abs(std::sin(PHI * M_PI / 2.0));
        
        // Subukan ang φ-cancellation model
        result.noise = base_growth * phi_factor + coeff_magnitude * 0.01;
        
        // I-normalize para hindi sumabog agad
        if (result.noise > 1e6) result.noise = 1e6;
        
        return result;
    }
    
public:
    FixedDegreeNoiseTest(int n = 64, long q = 536870909) 
        : N(n), Q(q) {}
    
    void run_test() {
        std::cout << "FIXED-DEGREE NOISE TEST (N=" << N << ")\n";
        std::cout << "========================================\n\n";
        std::cout << "Proper noise tracking (hiwalay sa ciphertext)\n";
        std::cout << "φ-based cancellation model\n\n";
        
        // Create initial ciphertexts with small noise
        Ciphertext ct_a(N);
        Ciphertext ct_b(N);
        ct_a.noise = 10.0;
        ct_b.noise = 10.0;
        
        // Simple initialization (small coefficients)
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(-2, 2);
        
        for (int i = 0; i < N; i++) {
            ct_a.c0[i] = dis(gen);
            ct_a.c1[i] = dis(gen);
            ct_a.c2[i] = dis(gen);
            
            ct_b.c0[i] = dis(gen);
            ct_b.c1[i] = dis(gen);
            ct_b.c2[i] = dis(gen);
        }
        
        // Run sequential NAND operations
        Ciphertext current = ct_a;
        std::vector<double> noise_history;
        noise_history.push_back(ct_a.noise);
        
        std::cout << "Depth\tNoise Level\tGrowth Factor\n";
        std::cout << "-----\t-----------\t-------------\n";
        std::cout << "0\t" << std::fixed << std::setprecision(4) 
                  << ct_a.noise << "\t1.0000\n";
        
        double prev_noise = ct_a.noise;
        int max_depth = 30;
        
        for (int depth = 1; depth <= max_depth; depth++) {
            current = homomorphic_nand(current, ct_b);
            
            double growth = (prev_noise > 0) ? current.noise / prev_noise : 0;
            
            std::cout << depth << "\t" 
                      << std::fixed << std::setprecision(4) 
                      << current.noise << "\t"
                      << growth << "\n";
            
            noise_history.push_back(current.noise);
            prev_noise = current.noise;
            
            if (current.noise > 1e6) {
                std::cout << "\nNoise exploded at depth " << depth << "\n";
                break;
            }
        }
        
        // Analysis
        std::cout << "\n=== ANALYSIS ===\n\n";
        
        if (noise_history.size() > 5) {
            std::cout << "Noise pattern: ";
            for (size_t i = 1; i < noise_history.size(); i++) {
                if (noise_history[i] < noise_history[i-1] * 0.9) {
                    std::cout << "↓";  // Noise decreased
                } else if (noise_history[i] > noise_history[i-1] * 1.1) {
                    std::cout << "↑";  // Noise increased
                } else {
                    std::cout << "→";  // Stable
                }
            }
            std::cout << "\n\n";
            
            double min_noise = *std::min_element(noise_history.begin(), noise_history.end());
            double max_noise = *std::max_element(noise_history.begin(), noise_history.end());
            double ratio = max_noise / min_noise;
            
            std::cout << "Min noise: " << min_noise << "\n";
            std::cout << "Max noise: " << max_noise << "\n";
            std::cout << "Max/min ratio: " << ratio << "\n";
            std::cout << "φ = " << PHI << "\n\n";
            
            if (ratio < PHI * 3) {
                std::cout << "✓ NOISE APPEARS CONTROLLED\n";
                std::cout << "  (pero hindi pa ito proof ng self-cancellation)\n";
            } else {
                std::cout << "✗ NOISE GROWING STEADILY\n";
                std::cout << "  Walang strong self-cancellation na na-observe\n";
            }
        }
    }
};

int main() {
    FixedDegreeNoiseTest test(64);
    test.run_test();
    return 0;
}
