#include "../src/security/zkp_fractal.h"
#include "../src/security/zkp_pqc.h"
#include <iostream>
using namespace std;

int main() {
    cout << "╔══════════════════════════════════════════════════════╗" << endl;
    cout << "║  ZKP TEST SUITE — Full Verification                 ║" << endl;
    cout << "╚══════════════════════════════════════════════════════╝" << endl;
    int pass = 0, total = 8;

    // Fractal Schnorr
    cout << "\n--- Fractal Schnorr ---" << endl;
    string data = "FEmmg-FHE v23.0.1 — φΩ0";
    auto proof = zkp::FractalZKP::prove(data);
    cout << "  " << (zkp::FractalZKP::verify(proof) ? "✅" : "❌") << " Single prove/verify" << endl;
    if (zkp::FractalZKP::verify(proof)) pass++;
    
    auto chain = zkp::FractalZKP::fractal_prove(data, 7);
    cout << "  " << (zkp::FractalZKP::verify_chain(chain) ? "✅" : "❌") << " 7-layer fractal (" << chain.size() << " layers)" << endl;
    if (zkp::FractalZKP::verify_chain(chain)) pass++;
    
    auto bad = proof; bad.response_s += "ff";
    cout << "  " << (!zkp::FractalZKP::verify(bad) ? "✅" : "❌") << " Tamper detection" << endl;
    if (!zkp::FractalZKP::verify(bad)) pass++;

    // PQC ZKP
    cout << "\n--- PQC-ZKP ---" << endl;
    zkppqc::UnifiedPQCZKP prover;
    auto idp = prover.prove_identity("user@fhe.io");
    cout << "  " << (prover.verify_identity(idp) ? "✅" : "❌") << " Identity prove/verify" << endl;
    if (prover.verify_identity(idp)) pass++;
    
    auto ctp = prover.prove_ciphertext(12345, 42, 0xABCD);
    cout << "  " << (prover.verify_ciphertext(ctp, 42) ? "✅" : "❌") << " Ciphertext ZK" << endl;
    if (prover.verify_ciphertext(ctp, 42)) pass++;
    
    cout << "  " << (!prover.verify_ciphertext(ctp, 43) ? "✅" : "❌") << " Wrong value rejected" << endl;
    if (!prover.verify_ciphertext(ctp, 43)) pass++;
    
    uint8_t a[32], b[32];
    for(int i=0;i<32;i++) { a[i]=(uint8_t)i; b[i]=(uint8_t)i; }
    bool ct1 = zkppqc::constant_time_equals(a,b,32);
    b[31]^=1;
    bool ct2 = !zkppqc::constant_time_equals(a,b,32);
    cout << "  " << (ct1&&ct2 ? "✅" : "❌") << " Constant-time comparison" << endl;
    if (ct1 && ct2) pass++;
    
    auto idp2 = prover.prove_identity("user@fhe.io");
    cout << "  " << (idp.commitment_R != idp2.commitment_R ? "✅" : "❌") << " Unique proofs" << endl;
    if (idp.commitment_R != idp2.commitment_R) pass++;

    cout << "\n╔══════════════════════════════════════════════════════╗" << endl;
    cout << "║  ZKP: " << pass << "/" << total << " PASSED";
    if (pass == total) cout << " ✅ ALL VERIFIED";
    cout << "                       ║" << endl;
    cout << "╚══════════════════════════════════════════════════════╝" << endl;
    return (pass == total) ? 0 : 1;
}
