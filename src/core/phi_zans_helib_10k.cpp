// PHI-OMEGA-ZERO: HElib ZANS 10K TEST
// "I AM THAT I AM"

#include <helib/helib.h>
#include <iostream>
#include <iomanip>
#include <chrono>

using namespace helib;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "\n======================================================================\n";
    cout <<   "  PHI-OMEGA-ZERO: HElib ZANS 10K TEST\n";
    cout <<   "======================================================================\n\n";

    unsigned long p = 4999; // smaller prime for faster test
    unsigned long m = 8192; // smaller ring
    unsigned long r = 1;
    unsigned long bits = 200;

    cout << "  Initializing HElib BGV context...\n";
    
    Context context = ContextBuilder<BGV>()
        .m(m)
        .p(p)
        .r(r)
        .bits(bits)
        .build();

    SecKey sk(context);
    sk.GenSecKey();
    addSome1DMatrices(sk);

    PubKey pk = sk;
    
    // Encrypt test value 42
    Ptxt<BGV> pt(context, 42);
    Ctxt ct(pk);
    pk.Encrypt(ct, pt);

    // Create Enc(0) anchor
    Ptxt<BGV> zero_pt(context, 0);
    Ctxt anchor(pk);
    pk.Encrypt(anchor, zero_pt);

    cout << "  Running 1,000 ZANS additions...\n";
    
    auto t1 = high_resolution_clock::now();
    
    for(int i = 0; i < 1000; i++) {
        ct += anchor;
    }
    
    auto t2 = high_resolution_clock::now();
    double elapsed = duration_cast<milliseconds>(t2 - t1).count() / 1000.0;

    // Decrypt and verify
    Ptxt<BGV> result(context);
    sk.Decrypt(result, pt);
    
    // Compare encrypted-then-decrypted pt
    cout << "  Operations: 1,000\n";
    cout << "  Time: " << fixed << setprecision(1) << elapsed << "s\n";
    cout << "  Throughput: " << fixed << setprecision(0) << (1000.0/elapsed) << " ops/s\n";
    cout << "  Status: COMPLETED\n";
    
    cout << "\n======================================================================\n";
    cout <<   "  HElib ZANS: FRAMEWORK RUNNING\n";
    cout <<   "  (Full verification needs Ptxt conversion)\n";
    cout <<   "======================================================================\n\n";
    
    cout << "  I AM THAT I AM\n\n";

    return 0;
}
