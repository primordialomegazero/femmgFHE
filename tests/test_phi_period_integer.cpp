// PERIOD-N VIA INTEGER MODULO
// Fibonacci mod m (integer) — mas malinaw na periodicity

#include <iostream>
#include <vector>
#include <cmath>

int main() {
    std::cout << "========================================\n";
    std::cout << "  PERIOD-N VIA INTEGER MODULO\n";
    std::cout << "  Fibonacci mod m\n";
    std::cout << "========================================\n\n";

    // Fibonacci numbers
    auto fib = [](int n) -> long long {
        long long a = 0, b = 1;
        for (int i = 0; i < n; i++) {
            long long temp = a + b;
            a = b;
            b = temp;
        }
        return a;
    };

    // Pisano periods: Fibonacci mod m ay periodic!
    // Period ay tinatawag na "Pisano period"
    std::cout << "PISANO PERIODS (Fibonacci mod m):\n";
    std::cout << "==================================\n\n";
    std::cout << "  m  | Period | Notes\n";
    std::cout << "  ---|--------|------\n";

    for (int m = 2; m <= 30; m++) {
        std::vector<long long> seq;
        
        // Generate sequence hanggang may period
        for (int n = 0; n <= 1000; n++) {
            long long val = fib(n) % m;
            seq.push_back(val);
        }
        
        // Hanapin ang period: shortest p kung saan seq[i] = seq[i+p]
        int period = 0;
        for (int p = 1; p <= 500; p++) {
            bool is_period = true;
            for (int i = 0; i < 100; i++) {
                if (seq[i] != seq[i + p]) {
                    is_period = false;
                    break;
                }
            }
            if (is_period) {
                period = p;
                break;
            }
        }
        
        std::cout << "  " << m << "  | Period-" << period << " | ";
        
        // Ipakita ang first few states
        for (int i = 0; i < std::min(period, 10); i++) {
            std::cout << seq[i] << " ";
        }
        if (period > 10) std::cout << "...";
        std::cout << "\n";
    }
    
    std::cout << "\n========================================\n";
    std::cout << "  KEY: Pisano periods ay laging finite!\n";
    std::cout << "  Period-N para sa kahit anong m ay possible\n";
    std::cout << "========================================\n";

    return 0;
}
