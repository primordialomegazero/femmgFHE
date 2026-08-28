// ============================================
// φ-DEBUG - TINGNAN ANG LOOB
// 
// Debug natin:
// 1. Saan nanggagaling ang error?
// 2. Ano ang nangyayari sa bawat operation?
// 3. Bakit may error sa multiplication?
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <random>
#include <chrono>
#include <algorithm>

using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-DEBUG - TINGNAN ANG LOOB\n";
    cout << "========================================\n\n";
    
    const double PHI = 1.6180339887498948482;
    const long long SCALE = 1000;
    
    cout << fixed << setprecision(15);
    
    // DEBUG 1: ENCODING PROCESS
    cout << "DEBUG 1: ENCODING PROCESS\n";
    cout << "=========================\n\n";
    
    double value = 3.0;
    double scaled = value * SCALE;
    
    cout << "  Value: " << value << "\n";
    cout << "  Scaled: " << scaled << "\n";
    
    long long b = (long long)round(scaled / PHI);
    long long a = (long long)round(scaled - b * PHI);
    
    cout << "  b = round(" << scaled << " / " << PHI << ") = " << b << "\n";
    cout << "  a = round(" << scaled << " - " << b << " * " << PHI << ") = " << a << "\n";
    
    double decoded = (a + b * PHI) / SCALE;
    cout << "  Decoded: " << decoded << "\n";
    cout << "  Error: " << abs(decoded - value) << "\n\n";
    
    // DEBUG 2: MULTIPLICATION PROCESS
    cout << "DEBUG 2: MULTIPLICATION PROCESS\n";
    cout << "===============================\n\n";
    
    // 3 × 7
    long long a1 = 0, b1 = 1854;  // 3.0
    long long a2 = 0, b2 = 4326;  // 7.0
    
    cout << "  3.0 = (" << a1 << ", " << b1 << ")\n";
    cout << "  7.0 = (" << a2 << ", " << b2 << ")\n\n";
    
    cout << "  Formula: (a1+b1φ)(a2+b2φ) = (a1a2+b1b2) + (a1b2+b1a2+b1b2)φ\n\n";
    
    long long new_a = (a1 * a2 + b1 * b2) / SCALE;
    long long new_b = (a1 * b2 + b1 * a2 + b1 * b2) / SCALE;
    
    cout << "  new_a = (" << a1 << "×" << a2 << " + " << b1 << "×" << b2 << ") / " << SCALE << "\n";
    cout << "        = (" << (a1 * a2) << " + " << (b1 * b2) << ") / " << SCALE << "\n";
    cout << "        = " << (a1 * a2 + b1 * b2) << " / " << SCALE << "\n";
    cout << "        = " << new_a << "\n\n";
    
    cout << "  new_b = (" << a1 << "×" << b2 << " + " << b1 << "×" << a2 << " + " << b1 << "×" << b2 << ") / " << SCALE << "\n";
    cout << "        = (" << (a1 * b2) << " + " << (b1 * a2) << " + " << (b1 * b2) << ") / " << SCALE << "\n";
    cout << "        = " << (a1 * b2 + b1 * a2 + b1 * b2) << " / " << SCALE << "\n";
    cout << "        = " << new_b << "\n\n";
    
    double result = (new_a + new_b * PHI) / SCALE;
    cout << "  Result: " << result << "\n";
    cout << "  Expected: 21\n";
    cout << "  Error: " << abs(result - 21.0) << "\n\n";
    
    // DEBUG 3: ANG SCALING ISSUE
    cout << "DEBUG 3: ANG SCALING ISSUE\n";
    cout << "==========================\n\n";
    
    cout << "  Ang problema: SCALE = " << SCALE << "\n";
    cout << "  Pero ang multiplication ay nagbibigay ng:\n";
    cout << "  new_a = (a1*a2 + b1*b2) / SCALE\n";
    cout << "  new_b = (a1*b2 + b1*a2 + b1*b2) / SCALE\n\n";
    
    cout << "  Kung a1=0, b1=1854, a2=0, b2=4326:\n";
    cout << "  new_a = (0 + 1854*4326) / 1000 = " << (1854.0 * 4326.0 / 1000.0) << "\n";
    cout << "  new_b = (0 + 0 + 1854*4326) / 1000 = " << (1854.0 * 4326.0 / 1000.0) << "\n\n";
    
    cout << "  Pero ang TAMANG formula ay:\n";
    cout << "  (0 + 1854φ/1000)(0 + 4326φ/1000)\n";
    cout << "  = (1854*4326/1000000)φ²\n";
    cout << "  = (1854*4326/1000000)(φ+1)\n";
    cout << "  = (1854*4326/1000000) + (1854*4326/1000000)φ\n\n";
    
    cout << "  Kaya ang TAMANG scaling ay:\n";
    cout << "  new_a = (a1*a2 + b1*b2) / (SCALE * SCALE)\n";
    cout << "  new_b = (a1*b2 + b1*a2 + b1*b2) / (SCALE * SCALE)\n\n";
    
    // DEBUG 4: TAMANG FORMULA
    cout << "DEBUG 4: TAMANG FORMULA\n";
    cout << "=======================\n\n";
    
    // Subukan ang tamang scaling
    long long correct_new_a = (a1 * a2 + b1 * b2) / (SCALE * SCALE);
    long long correct_new_b = (a1 * b2 + b1 * a2 + b1 * b2) / (SCALE * SCALE);
    
    double correct_result = (correct_new_a + correct_new_b * PHI) / SCALE;
    
    cout << "  With SCALE² scaling:\n";
    cout << "  new_a = " << correct_new_a << "\n";
    cout << "  new_b = " << correct_new_b << "\n";
    cout << "  Result: " << correct_result << "\n";
    cout << "  Expected: 21\n";
    cout << "  Error: " << abs(correct_result - 21.0) << "\n\n";
    
    // DEBUG 5: ALTERNATIVE - WALANG DIVISION
    cout << "DEBUG 5: ALTERNATIVE - WALANG DIVISION\n";
    cout << "=====================================\n\n";
    
    cout << "  Kung hindi natin i-divide sa SCALE:\n";
    cout << "  new_a = a1*a2 + b1*b2\n";
    cout << "  new_b = a1*b2 + b1*a2 + b1*b2\n\n";
    
    long long raw_a = a1 * a2 + b1 * b2;
    long long raw_b = a1 * b2 + b1 * a2 + b1 * b2;
    
    double raw_result = (raw_a + raw_b * PHI) / (SCALE * SCALE);
    
    cout << "  raw_a = " << raw_a << "\n";
    cout << "  raw_b = " << raw_b << "\n";
    cout << "  Result (divide by SCALE²): " << raw_result << "\n";
    cout << "  Expected: 21\n";
    cout << "  Error: " << abs(raw_result - 21.0) << "\n\n";
    
    // DEBUG 6: ANG TUNAY NA SAGOT
    cout << "DEBUG 6: ANG TUNAY NA SAGOT\n";
    cout << "===========================\n\n";
    
    cout << "  Ang φ-basis multiplication ay:\n";
    cout << "  (a1 + b1φ)(a2 + b2φ) = (a1a2 + b1b2) + (a1b2 + b1a2 + b1b2)φ\n\n";
    
    cout << "  Kung ang values ay naka-scale sa SCALE:\n";
    cout << "  value = (a + bφ) / SCALE\n\n";
    
    cout << "  Multiplication ng dalawang scaled values:\n";
    cout << "  [(a1 + b1φ)/SCALE] × [(a2 + b2φ)/SCALE]\n";
    cout << "  = [(a1+b1φ)(a2+b2φ)] / SCALE²\n";
    cout << "  = [(a1a2+b1b2) + (a1b2+b1a2+b1b2)φ] / SCALE²\n\n";
    
    cout << "  Para ma-represent sa SCALE basis:\n";
    cout << "  = [(a1a2+b1b2)/SCALE + (a1b2+b1a2+b1b2)φ/SCALE] / SCALE\n\n";
    
    cout << "  Kaya:\n";
    cout << "  new_a = (a1a2 + b1b2) / SCALE\n";
    cout << "  new_b = (a1b2 + b1a2 + b1b2) / SCALE\n\n";
    
    cout << "  PERO ito ay may rounding error!\n";
    cout << "  Ang eksaktong value ay:\n";
    cout << "  new_a_exact = (a1a2 + b1b2) / SCALE²\n";
    cout << "  new_b_exact = (a1b2 + b1a2 + b1b2) / SCALE²\n\n";
    
    cout << "  ANG SOLUSYON:\n";
    cout << "  1. Gumamit ng mas malaking SCALE\n";
    cout << "  2. O huwag i-divide (raw values)\n";
    cout << "  3. O gumamit ng arbitrary precision\n\n";
    
    return 0;
}
