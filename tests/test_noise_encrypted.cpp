#include "../src/core/femmg_operations.h"
#include <iostream>
#include <cmath>
using namespace std;

int main() {
    FEmmgFHE fhe;
    
    auto ct = fhe.encrypt(1);
    double min_n = ct.noise, max_n = ct.noise;
    
    for (int i = 0; i < 1000; i++) {
        auto one = fhe.encrypt(1);
        ct = fhe.add(ct, one);
        if (ct.noise < min_n) min_n = ct.noise;
        if (ct.noise > max_n) max_n = ct.noise;
    }
    
    cout << "Noise after 1000 encrypted adds:" << endl;
    cout << "  Min: " << min_n << endl;
    cout << "  Max: " << max_n << endl;
    cout << "  Drift: " << (max_n - min_n) << endl;
    cout << "  Expected: 1.82815" << endl;
    
    bool flatline = (abs(max_n - 1.82815) < 0.001 && abs(min_n - 1.82815) < 0.001);
    cout << "  FLATLINE: " << (flatline ? "✅" : "❌") << endl;
    
    int64_t result = fhe.decrypt(ct);
    cout << "  Value: " << result << " (expected " << (1 + 1000) << ")" << endl;
    cout << "  Correct: " << (result == 1001 ? "✅" : "❌") << endl;
    
    return 0;
}
