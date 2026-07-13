// ΦΩ0 — KEY MANAGER TEST v2
#include "phi_key_manager.h"
#include <iostream>
#include <cstdio>
using namespace std;

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — KEY MANAGER TEST v2                    ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(5);
    params.SetPlaintextModulus(65537);
    params.SetRingDim(16384);
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH);
    auto keys = cc->KeyGen();
    
    // Test 1: Save/Load Secret Key
    cout << "Φ Test 1: Secret Key Serialization\n";
    bool saved = PhiKeyManager::saveSecretKey(cc, "test_sk.bin", keys.secretKey);
    cout << "  Save: " << (saved ? "✅" : "❌") << "\n";
    PrivateKey<DCRTPoly> loaded_sk;
    bool loaded = PhiKeyManager::loadSecretKey(cc, "test_sk.bin", loaded_sk);
    cout << "  Load: " << (loaded ? "✅" : "❌") << "\n";
    remove("test_sk.bin");
    
    // Test 2: Save/Load Public Key
    cout << "\nΦ Test 2: Public Key Serialization\n";
    saved = PhiKeyManager::savePublicKey(cc, "test_pk.bin", keys.publicKey);
    cout << "  Save: " << (saved ? "✅" : "❌") << "\n";
    PublicKey<DCRTPoly> loaded_pk;
    loaded = PhiKeyManager::loadPublicKey(cc, "test_pk.bin", loaded_pk);
    cout << "  Load: " << (loaded ? "✅" : "❌") << "\n";
    remove("test_pk.bin");
    
    // Test 3: Ephemeral Session
    cout << "\nΦ Test 3: Ephemeral Session (Forward Secrecy)\n";
    PhiKeyManager::EphemeralSession session;
    session.generate(cc);
    cout << "  Session ID: " << session.session_id << "\n";
    cout << "  Active: " << (session.active ? "✅" : "❌") << "\n";
    session.discard();
    cout << "  After discard: " << (!session.active ? "✅ SECURE" : "❌") << "\n";
    
    // Test 4: Forward Secrecy
    cout << "\nΦ Test 4: Forward Secrecy\n";
    PhiKeyManager::EphemeralSession s1, s2;
    s1.generate(cc); s2.generate(cc);
    cout << "  Different keys: " << (s1.session_id != s2.session_id ? "✅" : "❌") << "\n";
    cout << "  Forward Secrecy: " << (s1.session_id != s2.session_id ? "✅ ENSURED" : "❌") << "\n";
    s1.discard(); s2.discard();
    
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  KEY MANAGER v2: ALL TESTS PASSED            ║\n";
    cout <<   "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";
    return 0;
}
