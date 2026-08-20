#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  CYCLOTOMIC FIELD Q(ζ_5) ANALYSIS\n";
    cout << "  May golden ratio na native\n";
    cout << "========================================\n\n";

    // 5th roots of unity
    // ζ_5 = e^(2πi/5) = cos(72°) + i·sin(72°)
    const double pi = acos(-1.0);
    complex<double> zeta5(cos(2*pi/5), sin(2*pi/5));
    complex<double> zeta5_2 = zeta5 * zeta5;
    complex<double> zeta5_3 = zeta5_2 * zeta5;
    complex<double> zeta5_4 = zeta5_3 * zeta5;

    cout << "1. 5TH ROOTS OF UNITY:\n";
    cout << "   ζ_5 = " << zeta5 << "\n";
    cout << "   ζ_5² = " << zeta5_2 << "\n";
    cout << "   ζ_5³ = " << zeta5_3 << "\n";
    cout << "   ζ_5⁴ = " << zeta5_4 << "\n";
    cout << "   ζ_5⁵ = " << (zeta5_4 * zeta5) << " (dapat 1)\n\n";

    cout << "2. GOLDEN RATIO SA Q(ζ_5):\n";
    cout << "   φ = ζ_5 + ζ_5⁴ + 1\n";
    complex<double> phi = zeta5 + zeta5_4 + 1.0;
    cout << "   φ = " << phi << "\n";
    cout << "   Real na φ = " << (1.0 + sqrt(5.0)) / 2.0 << "\n";
    cout << "   Match: " << (abs(phi.real() - (1.0 + sqrt(5.0))/2.0) < 1e-10 ? "YES" : "NO") << "\n\n";

    cout << "3. √5 SA Q(ζ_5):\n";
    cout << "   √5 = 2ζ_5 + 2ζ_5⁴ + 1\n";
    complex<double> sqrt5 = 2.0*zeta5 + 2.0*zeta5_4 + 1.0;
    cout << "   √5 = " << sqrt5 << "\n";
    cout << "   Expected: " << sqrt(5.0) << "\n";
    cout << "   Match: " << (abs(sqrt5.real() - sqrt(5.0)) < 1e-10 ? "YES" : "NO") << "\n\n";

    cout << "4. MINIMAL POLYNOMIAL NG ζ_5:\n";
    cout << "   Φ_5(x) = x⁴ + x³ + x² + x + 1\n";
    cout << "   Ito ay degree 4\n";
    cout << "   (Hindi power of 2!)\n\n";

    cout << "5. ANG RING Z[ζ_5]:\n";
    cout << "   Degree: 4 (hindi 2^k)\n";
    cout << "   Units: infinite group na may φ\n";
    cout << "   φ ay unit sa Z[ζ_5]\n";
    cout << "   Dahil φ × (1/φ) = 1\n";
    cout << "   at 1/φ = φ - 1 = ζ_5 + ζ_5⁴\n\n";

    cout << "6. NOISE GROWTH SA Z[ζ_5]:\n";
    cout << "   Ang noise sa Z[ζ_5] ay may\n";
    cout << "   DIFFERENT structure kaysa Z[X]/(X^N+1)\n";
    cout << "   Dahil ang degree ay 4 (hindi power of 2)\n";
    cout << "   Ang coefficient growth ay mas mabagal\n\n";

    cout << "7. CYCLOTOMIC PROPERTIES:\n";
    cout << "   Φ_5(x) ay irreducible over Q\n";
    cout << "   Ang discriminant ay 125\n";
    cout << "   Ang ramified prime ay 5 lang\n";
    cout << "   (Sa Z[X]/(X^N+1), ang ramified prime ay 2)\n\n";

    cout << "========================================\n";
    cout << "  KEY INSIGHT:\n";
    cout << "  - Q(ζ_5) ay may golden ratio na native\n";
    cout << "  - Degree 4, hindi power of 2\n";
    cout << "  - Ramified prime ay 5, hindi 2\n";
    cout << "  - Ang noise structure ay FUNDAMENTALLY\n";
    cout << "    DIFFERENT sa BFV\n";
    cout << "========================================\n";

    return 0;
}
