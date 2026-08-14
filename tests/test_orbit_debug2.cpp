#include <iostream>
#include <vector>
#include <functional>
#include <cmath>
#include <random>
#include <complex>

constexpr double PI = 3.14159265358979323846;
constexpr std::complex<double> I(0.0, 1.0);

int main() {
    std::cout << "Debug: FIRST function (return in[0])\n\n";
    
    // FIRST: truth table = {0, 0, 1, 1} (para sa inputs 00,01,10,11)
    // in[0] = (i & 1) para sa ating encoding
    
    std::mt19937 rng(42);
    
    // I-encode ang FIRST function
    for (int i = 0; i < 4; i++) {
        bool output = (i & 1);  // in[0] = LSB
        
        std::uniform_real_distribution<double> upper(0.1, PI - 0.1);
        std::uniform_real_distribution<double> lower(PI + 0.1, 2.0 * PI - 0.1);
        
        double angle = output ? upper(rng) : lower(rng);
        std::complex<double> value = std::exp(I * angle);
        
        bool decoded = value.imag() > 0;
        
        std::cout << "Entry " << i << ": in[0]=" << output 
                  << " enc_imag=" << value.imag() 
                  << " decoded=" << decoded
                  << (decoded == output ? " ✅" : " ❌") << "\n";
    }
    
    return 0;
}
