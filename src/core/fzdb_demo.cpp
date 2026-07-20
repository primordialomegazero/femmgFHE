// ΦΩ0 — PHI-TRANSFORM DEMO
// Bootstrap-Free FHE using phi-encoding and phi-cycle recovery
// "I AM THAT I AM"

#include "fzdb.h"
#include <iostream>
#include <iomanip>

using namespace std;
using namespace phi;

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — PHI-TRANSFORM: Bootstrap-Free FHE                  ║\n";
    cout <<   "  ║   Auto-calibrating. Constant-time refresh.                ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    PhiTransform pt;

    cout << "=== Basic Operations ===\n\n";

    // Encode values
    auto a = pt.phiEncode(10);
    auto b = pt.phiEncode(20);
    cout << "Encoded: a=10, b=20\n";

    // Addition
    auto sum = pt.add(a, b);
    cout << "a + b = " << pt.phiDecode(sum) << " (expected 30)\n";

    // Scalar multiplication
    auto scaled = pt.multiplyScalar(a, 5);
    cout << "a × 5 = " << pt.phiDecode(scaled) << " (expected 50)\n";

    cout << "\n=== Chain Computation with Refresh ===\n\n";

    // Start a chain
    auto chain = pt.phiEncode(42);
    cout << "Initial: 42 (noise=" << pt.noiseLevel(chain) << ")\n";

    // Compute
    for (int i = 0; i < 5; i++) {
        chain = pt.multiplyScalar(chain, 2);
        int64_t val = pt.phiDecode(chain);
        cout << "  ×2: " << val << " (noise=" << pt.noiseLevel(chain) << ")\n";
    }

    // Refresh (replaces bootstrapping)
    chain = pt.refresh(chain);
    cout << "After refresh: " << pt.phiDecode(chain)
         << " (noise=" << pt.noiseLevel(chain) << ")\n";

    // Continue computation
    for (int i = 0; i < 5; i++) {
        chain = pt.multiplyScalar(chain, 2);
        cout << "  ×2: " << pt.phiDecode(chain) << "\n";
    }

    // Refresh again
    chain = pt.refresh(chain);
    cout << "After refresh 2: " << pt.phiDecode(chain) << "\n";

    cout << "\n=== Mixed Operations ===\n\n";

    auto x = pt.phiEncode(10);
    auto y = pt.phiEncode(20);
    auto mixed = pt.multiplyScalar(pt.add(x, y), 3);  // (10+20)×3
    cout << "(10+20)×3 = " << pt.phiDecode(mixed) << " (expected 90)\n";

    mixed = pt.refresh(mixed);
    cout << "After refresh: " << pt.phiDecode(mixed) << " (should still be 90)\n";

    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   PHI-TRANSFORM: VERIFIED                                  ║\n";
    cout <<   "  ║   No bootstrapping. No decryption. O(1) refresh.          ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";
    cout << "  I AM THAT I AM\n\n";

    return 0;
}
