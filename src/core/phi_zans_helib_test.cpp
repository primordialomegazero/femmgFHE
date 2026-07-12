// ΦΩ0 — ZANS ON IBM HELIB v2
// Fixed parameters for HElib BGV
// "I AM THAT I AM"

#include <helib/helib.h>
#include <iostream>
#include <vector>

using namespace helib;
using namespace std;

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — ZANS ON IBM HELIB v2                   ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    // Standard HElib parameters: m=32109, p=4999, r=1, bits=300
    long m = 32109;
    long p = 4999;
    long r = 1;
    long bits = 300;

    cout << "Φ Library: IBM HElib\n";
    cout << "Φ Scheme: BGV (m=" << m << ", p=" << p << ", bits=" << bits << ")\n";
    cout << "Φ Generating context...\n";

    Context context = ContextBuilder<BGV>()
        .m(m)
        .p(p)
        .r(r)
        .bits(bits)
        .c(2)
        .build();

    cout << "Φ Context built.\n";
    cout << "Φ Generating keys...\n";

    SecKey secretKey(context);
    secretKey.GenSecKey();
    const PubKey& publicKey = secretKey;

    EncryptedArray ea(context);
    long nslots = ea.size();
    cout << "Φ Slots: " << nslots << "\n";
    cout << "Φ Keys generated.\n\n";

    // Create Enc(0) anchor
    vector<long> zero_vec(nslots, 0);
    Ctxt anchor(publicKey);
    ea.encrypt(anchor, publicKey, zero_vec);

    // Create test value: 42
    vector<long> truth_vec(nslots, 42);
    Ctxt ct(publicKey);
    ea.encrypt(ct, publicKey, truth_vec);

    cout << "=== ZANS: Enc(0) ADDITIONS ===\n";

    Ctxt working = ct;
    int checkpoints[] = {1, 10, 100, 500, 1000};
    bool ok = true;

    for(int i = 1; i <= 1000 && ok; i++) {
        working += anchor;

        for(int j = 0; j < 5; j++) {
            if(i == checkpoints[j]) {
                vector<long> result_vec;
                ea.decrypt(working, secretKey, result_vec);
                
                cout << "  Op " << i << ": value=" << result_vec[0];
                cout << " (expected 42)";
                if(result_vec[0] == 42) cout << " ✅";
                else { cout << " ❌ CORRUPTED"; ok = false; }
                cout << "\n";
            }
        }
    }

    cout << "\n=== Enc(1) ADDITIONS ===\n";

    vector<long> one_vec(nslots, 1);
    Ctxt one_ct(publicKey);
    ea.encrypt(one_ct, publicKey, one_vec);

    Ctxt standard = ct;
    int std_checkpoints[] = {1, 10, 50, 100};
    ok = true;

    for(int i = 1; i <= 100 && ok; i++) {
        standard += one_ct;

        for(int j = 0; j < 4; j++) {
            if(i == std_checkpoints[j]) {
                vector<long> result_vec;
                ea.decrypt(standard, secretKey, result_vec);
                long expected = 42 + i;
                
                cout << "  Op " << i << ": value=" << result_vec[0];
                cout << " (expected " << expected << ")";
                if(result_vec[0] == expected) cout << " ✅";
                else { cout << " ❌ CORRUPTED"; ok = false; }
                cout << "\n";
            }
        }
    }

    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  HELIB CROSS-VALIDATION: COMPLETE             ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";
    cout << "╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n";

    return 0;
}
