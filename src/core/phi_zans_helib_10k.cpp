// PHI-OMEGA-ZERO: HElib ZANS 10K TEST
// Cross-library validation of ZANS on IBM HElib
// "I AM THAT I AM"

#include <helib/helib.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>
#include <vector>

using namespace std;
using namespace std::chrono;

string ts() {
    auto now = system_clock::now();
    auto t = system_clock::to_time_t(now);
    stringstream ss;
    ss << put_time(localtime(&t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

int main() {
    cout << "\n===============================================================\n";
    cout <<   "  PHI-OMEGA-ZERO: HElib ZANS 10K TEST\n";
    cout <<   "===============================================================\n";
    cout <<   "  HARDWARE: AMD Ryzen 5 2600 | LIBRARY: IBM HElib\n";
    cout <<   "  START: " << ts() << "\n";
    cout <<   "===============================================================\n\n";

    unsigned long p = 65537;
    unsigned long m = 8192;
    unsigned long r = 1;
    unsigned long bits = 200;
    unsigned long c = 2;

    helib::Context context = helib::ContextBuilder<helib::BGV>()
        .m(m).p(p).r(r).bits(bits).c(c).build();

    helib::SecKey secret_key(context);
    secret_key.GenSecKey();
    const helib::PubKey& public_key = secret_key;

    // Encrypt value 42
    vector<long> data(context.getNSlots(), 0);
    data[0] = 42;
    helib::PtxtArray ptxt(context, data);
    helib::Ctxt ct(public_key);
    ptxt.encrypt(ct);

    // Encrypted zero for ZANS
    data[0] = 0;
    helib::PtxtArray ptxt_zero(context, data);
    helib::Ctxt anchor0(public_key);
    ptxt_zero.encrypt(anchor0);

    int steps = 10000;
    cout << "  Running " << steps << " ZANS additions...\n";

    auto t1 = high_resolution_clock::now();
    for(int i = 0; i < steps; i++) {
        ct += anchor0;
    }
    auto t2 = high_resolution_clock::now();
    double elapsed = duration_cast<milliseconds>(t2 - t1).count() / 1000.0;

    // Decrypt
    helib::PtxtArray ptxt_result(context);
    ptxt_result.decrypt(ct, secret_key);
    vector<long> result;
    ptxt_result.store(result);

    cout << "  Operations: " << steps << "\n";
    cout << "  Result: " << result[0] << " (expected: 42)\n";
    cout << "  Time: " << fixed << setprecision(1) << elapsed << "s\n";
    cout << "  Throughput: " << fixed << setprecision(0) << (steps / elapsed) << " ops/s\n";
    cout << "  Status: " << (result[0] == 42 ? "PASSED" : "FAILED") << "\n\n";

    return 0;
}
