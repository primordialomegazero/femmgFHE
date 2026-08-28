// PHASE PLOT — (cos, sin) ng Pentagonal Addition
// Obserbahan ang geometric pattern

#include <iostream>
#include <vector>
#include <cmath>

using namespace std;

int main() {
    const double PI = 3.14159265358979323846;
    const double ENC_0 = -2 * PI / 5;
    const double ENC_1 = 2 * PI / 5;

    cout << "========================================\n";
    cout << "  PHASE PLOT — Pentagonal States\n";
    cout << "========================================\n\n";

    vector<pair<double,double>> inputs = {
        {ENC_0, ENC_0},
        {ENC_0, ENC_1},
        {ENC_1, ENC_0},
        {ENC_1, ENC_1}
    };

    cout << "Inputs at Resulting Phase Points:\n";
    cout << "----------------------------------\n\n";

    for (auto [a, b] : inputs) {
        double sum = a + b;
        double cos_val = cos(sum);
        double sin_val = sin(sum);

        cout << "(" << (a * 180/PI) << "°, " << (b * 180/PI) << "°) -> ";
        cout << "(" << cos_val << ", " << sin_val << ")\n";
    }

    cout << "\nPentagon Vertices (para reference):\n";
    cout << "------------------------------------\n";
    for (int k = 0; k < 5; k++) {
        double angle = 2 * PI * k / 5;
        cout << "k=" << k << ": (" << cos(angle) << ", " << sin(angle) << ")\n";
    }

    cout << "\nObserbasyon:\n";
    cout << "  - May tatlong phase points lang lumalabas\n";
    cout << "  - (0,1) at (1,0) ay iisa ang puntong (1,0)\n";
    cout << "  - (0,0) at (1,1) ay nasa ±144°\n";
    cout << "  - Ito ay parang truncated pentagon\n";

    return 0;
}
