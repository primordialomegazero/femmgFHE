// ZECKENDORF V2 — Tama nang representation
// Walang F(0), simula sa F(1)

#include <iostream>
#include <vector>

int main() {
    std::cout << "========================================\n";
    std::cout << "  ZECKENDORF V2 — Correct Representation\n";
    std::cout << "========================================\n\n";

    long long F[20];
    F[0] = 0; F[1] = 1;
    for (int i = 2; i < 20; i++) F[i] = F[i-1] + F[i-2];

    std::cout << "ZECKENDORF (walang F(0)):\n";
    std::cout << "========================\n\n";

    for (int n = 0; n <= 25; n++) {
        std::cout << "  " << n << " = ";
        std::vector<int> fibs;
        int temp = n;
        for (int i = 14; i >= 1; i--) {  // Simula sa F(1)
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

    std::cout << "\nF(1) PATTERN:\n";
    std::cout << "=============\n\n";
    for (int n = 0; n <= 25; n++) {
        int temp = n;
        bool has_f1 = false;
        for (int i = 14; i >= 1; i--) {
            if (F[i] <= temp) {
                if (i == 1) has_f1 = true;
                temp -= F[i];
            }
        }
        std::cout << "  " << n << " → " << (has_f1 ? "1" : "0") << "\n";
    }

    return 0;
}
