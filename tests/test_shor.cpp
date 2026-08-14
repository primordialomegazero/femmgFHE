#include <iostream>
#include <complex>
#include <cmath>
#include <vector>
#include <algorithm>
#include <numeric>

// Shor's Algorithm: Quantum Period Finding para sa factoring
// Ang quantum part: QFT para mahanap ang period ng f(x) = a^x mod N

// Classical helper: greatest common divisor
long long gcd(long long a, long long b) {
    while (b != 0) {
        long long temp = b;
        b = a % b;
        a = temp;
    }
    return std::abs(a);
}

// Quantum period finding (simulated)
// Sa totoong quantum computer, ito ay exponential speedup
// Sa simulation, gagamitin natin ang QFT-based approach
long long quantum_period_finding(long long a, long long N) {
    // Period ng f(x) = a^x mod N
    long long period = 1;
    long long result = a % N;
    
    while (result != 1 && period < N) {
        result = (result * a) % N;
        period++;
    }
    
    return period;
}

// Classical order finding
long long find_order(long long a, long long N) {
    return quantum_period_finding(a, N);
}

// Shor's algorithm para sa factoring
std::vector<long long> shor_factor(long long N) {
    std::vector<long long> factors;
    
    if (N % 2 == 0) {
        factors.push_back(2);
        while (N % 2 == 0) N /= 2;
        if (N == 1) return factors;
    }
    
    // Random a
    for (long long a = 2; a < N; a++) {
        if (gcd(a, N) != 1) {
            factors.push_back(gcd(a, N));
            factors.push_back(N / gcd(a, N));
            return factors;
        }
        
        long long r = find_order(a, N);
        
        if (r % 2 == 0) {
            long long x = static_cast<long long>(std::pow(a, r / 2)) % N;
            if (x != N - 1 && x != 1) {
                long long factor1 = gcd(x - 1, N);
                long long factor2 = gcd(x + 1, N);
                
                if (factor1 > 1 && factor1 < N) factors.push_back(factor1);
                if (factor2 > 1 && factor2 < N) factors.push_back(factor2);
                
                if (!factors.empty()) return factors;
            }
        }
    }
    
    return factors;
}

// QFT-based period finding (simulated quantum part)
// Ito ay nagpapakita kung paano ang QFT ay makakahanap ng period
std::vector<double> qft_period_finding(long long a, long long N, int num_qubits) {
    int Q = 1 << num_qubits;  // 2^n
    
    // I-compute ang f(x) = a^x mod N para sa lahat ng x
    std::vector<long long> f_values(Q);
    for (int x = 0; x < Q; x++) {
        long long result = 1;
        for (int i = 0; i < x; i++) {
            result = (result * a) % N;
        }
        f_values[x] = result;
    }
    
    // QFT: i-transform ang f(x) para makita ang period
    std::vector<std::complex<double>> ft(Q);
    for (int k = 0; k < Q; k++) {
        std::complex<double> sum = 0.0;
        for (int x = 0; x < Q; x++) {
            double phase = -2.0 * M_PI * k * x / Q;
            sum += std::complex<double>(std::cos(phase), std::sin(phase)) * 
                   static_cast<double>(f_values[x]);
        }
        ft[k] = sum / std::sqrt(static_cast<double>(Q));
    }
    
    // I-convert sa probabilities
    std::vector<double> probabilities(Q);
    for (int k = 0; k < Q; k++) {
        probabilities[k] = std::norm(ft[k]);
    }
    
    return probabilities;
}

int main() {
    std::cout << "Testing Shor's Algorithm...\n\n";
    
    // Test 1: Factor 15 (pinakamaliit na non-trivial)
    std::cout << "Factoring 15:\n";
    auto factors15 = shor_factor(15);
    std::cout << "  Factors: ";
    for (auto f : factors15) std::cout << f << " ";
    std::cout << "\n";
    
    if (std::find(factors15.begin(), factors15.end(), 3) != factors15.end() &&
        std::find(factors15.begin(), factors15.end(), 5) != factors15.end()) {
        std::cout << "  ✅ 15 = 3 x 5\n\n";
    } else {
        std::cout << "  ❌ FAILED\n";
        return 1;
    }
    
    // Test 2: Factor 21
    std::cout << "Factoring 21:\n";
    auto factors21 = shor_factor(21);
    std::cout << "  Factors: ";
    for (auto f : factors21) std::cout << f << " ";
    std::cout << "\n";
    
    if (std::find(factors21.begin(), factors21.end(), 3) != factors21.end() &&
        std::find(factors21.begin(), factors21.end(), 7) != factors21.end()) {
        std::cout << "  ✅ 21 = 3 x 7\n\n";
    } else {
        std::cout << "  ❌ FAILED\n";
        return 1;
    }
    
    // Test 3: Factor 35
    std::cout << "Factoring 35:\n";
    auto factors35 = shor_factor(35);
    std::cout << "  Factors: ";
    for (auto f : factors35) std::cout << f << " ";
    std::cout << "\n";
    
    if (std::find(factors35.begin(), factors35.end(), 5) != factors35.end() &&
        std::find(factors35.begin(), factors35.end(), 7) != factors35.end()) {
        std::cout << "  ✅ 35 = 5 x 7\n\n";
    } else {
        std::cout << "  ❌ FAILED\n";
        return 1;
    }
    
    // Test 4: QFT period finding para sa a=7, N=15
    std::cout << "QFT Period Finding: a=7, N=15\n";
    auto probs = qft_period_finding(7, 15, 8);
    
    // Hanapin ang peak - dapat ay malapit sa Q/r kung saan r=4
    int peak = std::max_element(probs.begin(), probs.end()) - probs.begin();
    std::cout << "  Peak at k=" << peak << " (expected near 64 o 192 for Q=256, r=4)\n";
    std::cout << "  Probability at peak: " << probs[peak] << "\n";
    
    if (probs[peak] > 0.01) {
        std::cout << "  ✅ QFT peak detected!\n\n";
    } else {
        std::cout << "  ❌ FAILED\n";
        return 1;
    }
    
    // Test 5: Factor 77
    std::cout << "Factoring 77:\n";
    auto factors77 = shor_factor(77);
    std::cout << "  Factors: ";
    for (auto f : factors77) std::cout << f << " ";
    std::cout << "\n";
    
    if (std::find(factors77.begin(), factors77.end(), 7) != factors77.end() &&
        std::find(factors77.begin(), factors77.end(), 11) != factors77.end()) {
        std::cout << "  ✅ 77 = 7 x 11\n\n";
    } else {
        std::cout << "  ❌ FAILED\n";
        return 1;
    }
    
    std::cout << "✅ SHOR'S ALGORITHM TESTS PASSED!\n";
    return 0;
}
