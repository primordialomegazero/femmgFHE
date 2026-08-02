// ΦΩ0 — V5.1 TRUE CT×CT: RING=32768
// Unlimited arbitrary ciphertext multiplication
// "IF PHYSICS ALLOWS IT, WE PUSH. IF NOT, WE FIND THE LIMIT."

#include "../src/core/phi_self_healing_fhe_v5.h"
#include <iostream>
#include <chrono>
#include <cmath>
using namespace std;
using namespace std::chrono;

int main() {
    int max_steps = 300;
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — TRUE CTxCT: " << max_steps << " mults, RING=32768             ║\n";
    cout <<   "  ║   Each step: ct_new = ct_curr * ct_prev             ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n";
    
    // ringDim=2^15=32768, 30 levels after boot, threshold=20 mults
    SelfHealingFHEv5 fhe(15, 30, 20);
    
    auto ct_prev = fhe.add_input("ct0", 2.0);
    auto ct_curr = fhe.add_input("ct1", 1.0001);
    double exp_prev = 2.0, exp_curr = 1.0001;
    
    for (int i = 0; i < max_steps; i++) {
        int mul_node = fhe.add_mul("ctxct_"+to_string(i), ct_curr, ct_prev);
        ct_prev = ct_curr;
        ct_curr = mul_node;
        double new_exp = exp_curr * exp_prev;
        exp_prev = exp_curr;
        exp_curr = new_exp;
    }
    
    auto t0 = high_resolution_clock::now();
    fhe.execute_no_verify();
    auto t1 = high_resolution_clock::now();
    
    double got = fhe.decrypt_value(fhe.get_node(ct_curr).ct);
    double err = abs((got - exp_curr) / exp_curr);
    
    cout << "  Final error: " << err 
         << " | Bootstraps: " << fhe.get_bootstrap_count()
         << " | Time: " << duration_cast<seconds>(t1-t0).count() << "s\n";
    
    bool pass = err < 0.01;
    cout << "  Result: " << (pass ? "PASSED — TRUE CTxCT WITH RING=32768" : "FAILED") << "\n\n";
    return pass ? 0 : 1;
}
