// PERIOD-24 STRICT VERIFICATION
// I-print lahat ng 24 states, i-check kung natatangi

#include <iostream>
#include <vector>
#include <cmath>
#include <set>

int main() {
    std::cout << "========================================\n";
    std::cout << "  PERIOD-24 STRICT VERIFICATION\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double nine_phi_sq = 9 * phi_sq;

    // Fibonacci sequence
    std::vector<double> fib = {0, 1};
    for (int i = 2; i <= 24; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    // I-print at i-verify LAHAT ng 24 states
    std::cout << "LAHAT NG 24 STATES (walang skip):\n";
    std::cout << "=================================\n\n";

    std::vector<double> states;
    std::set<int> unique_states;

    for (int i = 0; i < 24; i++) {
        double state_val = std::fmod(fib[i] * phi_sq, nine_phi_sq);
        states.push_back(state_val);
        
        // I-round para sa uniqueness check
        int rounded = (int)(state_val * 1000);
        unique_states.insert(rounded);
        
        std::cout << "  State " << i << ": " << state_val << "\n";
    }

    std::cout << "\n";

    // Check uniqueness
    std::cout << "UNIQUENESS CHECK:\n";
    std::cout << "=================\n\n";
    std::cout << "  Total states: " << states.size() << "\n";
    std::cout << "  Unique states: " << unique_states.size() << "\n";
    std::cout << "  May repeats ba bago ang 24? " 
              << (unique_states.size() < states.size() ? "OO" : "WALA") << "\n\n";

    // Check kung may repeat sa mas mababang step
    std::cout << "REPEAT DETECTION:\n";
    std::cout << "=================\n\n";

    bool has_early_repeat = false;
    for (int i = 0; i < 24; i++) {
        for (int j = i + 1; j < 24; j++) {
            if (std::abs(states[i] - states[j]) < 0.001) {
                std::cout << "  Repeat: State " << i << " = State " << j 
                          << " (" << states[i] << ")\n";
                has_early_repeat = true;
            }
        }
    }

    if (!has_early_repeat) {
        std::cout << "  Walang repeat sa loob ng 24 steps!\n";
        std::cout << "  ✅ TUNAY NA PERIOD-24!\n";
    }

    std::cout << "\n";

    // Verify na babalik sa State 0 pagkatapos ng 24 steps
    double state_24 = std::fmod(fib[24] * phi_sq, nine_phi_sq);
    std::cout << "State 24 (pagkatapos ng 1 cycle):\n";
    std::cout << "  " << state_24 << " (dapat 0)\n";
    std::cout << "  " << (std::abs(state_24) < 0.001 ? "✅ BUMALIK SA 0!" : "❌ HINDI BUMALIK") << "\n\n";

    // State 25 = State 1?
    double state_25 = std::fmod(fib[25] * phi_sq, nine_phi_sq);
    std::cout << "State 25 (dapat = State 1):\n";
    std::cout << "  " << state_25 << " (dapat " << states[1] << ")\n";
    std::cout << "  " << (std::abs(state_25 - states[1]) < 0.001 ? "✅ MATCH!" : "❌ MISMATCH") << "\n\n";

    std::cout << "========================================\n";
    std::cout << "  FINAL VERDICT:\n";
    std::cout << "  " << (!has_early_repeat ? "✅ TUNAY NA PERIOD-24" : "❌ MAY REPEAT") << "\n";
    std::cout << "========================================\n";

    return 0;
}
