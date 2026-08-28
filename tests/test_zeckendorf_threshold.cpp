// ZECKENDORF THRESHOLD — Natural Binary
// Pure mathematics, walang OpenFHE

#include <iostream>
#include <vector>

int main() {
    std::cout << "========================================\n";
    std::cout << "  ZECKENDORF THRESHOLD\n";
    std::cout << "  Natural Binary Representation\n";
    std::cout << "========================================\n\n";

    // Fibonacci numbers
    long long F[20];
    F[0] = 0; F[1] = 1;
    for (int i = 2; i < 20; i++) F[i] = F[i-1] + F[i-2];

    std::cout << "FIBONACCI NUMBERS:\n";
    for (int i = 0; i < 15; i++) std::cout << "  F(" << i << ")=" << F[i];
    std::cout << "\n\n";

    // Zeckendorf representation
    std::cout << "ZECKENDORF REPRESENTATION:\n";
    std::cout << "==========================\n\n";
    
    for (int n = 0; n <= 20; n++) {
        std::cout << "  " << n << " = ";
        std::vector<int> fibs;
        int temp = n;
        for (int i = 14; i >= 0; i--) {
            if (F[i] <= temp) {
                fibs.push_back(i);
                temp -= F[i];
            }
        }
        if (fibs.empty()) std::cout << "0";
        for (int j = 0; j < fibs.size(); j++) {
            std::cout << "F(" << fibs[j] << ")";
            if (j < fibs.size() - 1) std::cout << " + ";
        }
        std::cout << "\n";
    }

    std::cout << "\nNATURAL THRESHOLD PATTERN:\n";
    std::cout << "==========================\n\n";
    std::cout << "  Tingnan ang F(1) sa representation:\n";
    std::cout << "  Kung may F(1) → odd na representation → 1\n";
    std::cout << "  Kung walang F(1) → even → 0\n\n";

    for (int n = 0; n <= 20; n++) {
        int temp = n;
        bool has_f1 = false;
        for (int i = 14; i >= 0; i--) {
            if (F[i] <= temp) {
                if (i == 1) has_f1 = true;
                temp -= F[i];
            }
        }
        std::cout << "  " << n << " → " << (has_f1 ? "1" : "0") << "\n";
    }

    std::cout << "\nPATTERN:\n";
    std::cout << "========\n\n";
    std::cout << "  0,1,0,1,0,1,0,1...\n";
    std::cout << "  Ito ay natural na toggle!\n";
    std::cout << "  Walang threshold — ang Fibonacci position ang nagde-decide.\n\n";

    return 0;
}
