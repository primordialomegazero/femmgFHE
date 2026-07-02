#include "../src/math/anti_lattice.h"
#include <iostream>
using namespace std;
using namespace anti_lattice;

int main() {
    cout << "======================================================" << endl;
    cout << "  ANTI-LATTICE TEST — All 4 Layers" << endl;
    cout << "======================================================" << endl;
    
    int pass = 0, total = 5;
    
    // 1. Information-Theoretic layer
    InfoTheoreticLayer it;
    uint8_t plain[] = {0xDE, 0xAD, 0xBE, 0xEF};
    uint8_t cipher[4], decrypted[4];
    it.generate_pad(4, 42);
    it.encrypt(plain, cipher, 4);
    it.decrypt(cipher, decrypted, 4);
    bool it_ok = (memcmp(plain, decrypted, 4) == 0);
    cout << "\n1. Info-Theoretic (OTP + φ): " << (it_ok ? "✅" : "❌") << endl;
    if (it_ok) pass++;
    
    // 2. Coding-Theory layer
    CodingTheoryLayer coding;
    vector<uint8_t> msg = {1,0,1,0,1,1,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0};
    auto cw = coding.encode(msg);
    coding.add_errors(cw, 3, 123);
    auto dec = coding.decode(cw, 3);
    bool ct_ok = (dec[0] == msg[0] && dec[1] == msg[1]);
    cout << "2. Coding-Theory (McEliece): " << (ct_ok ? "✅" : "❌") << endl;
    if (ct_ok) pass++;
    
    // 3. MQ Layer
    MQLayer mq;
    vector<int64_t> mq_in = {1,2,3,4,5,6,7,8};
    auto mq_out = mq.evaluate(mq_in);
    bool mq_ok = (mq_out.size() == MQ_EQS);
    cout << "3. MQ Equations (" << MQ_VARS << " vars, " << MQ_EQS << " eqs): " << (mq_ok ? "✅" : "❌") << endl;
    if (mq_ok) pass++;
    
    // 4. Hash-Based layer
    HashBasedLayer hb;
    uint8_t secret[] = {0xFE, 0xDC, 0xBA, 0x98};
    auto chain = hb.generate_chain(secret, 4, 99);
    bool hb_ok = hb.verify_chain(chain, 99);
    cout << "4. Hash-Based (Lamport chain): " << (hb_ok ? "✅" : "❌") << endl;
    if (hb_ok) pass++;
    
    // 5. Unified Engine
    AntiLatticeEngine ale;
    vector<uint8_t> pt = {0x42, 0x42, 0x42, 0x42};
    auto ct = ale.encrypt(pt, 777);
    bool unified_ok = (ct.size() > pt.size());  // Ciphertext should be larger (encoding + errors)
    cout << "5. Unified Anti-Lattice: " << (unified_ok ? "✅" : "❌") << " (ct size: " << ct.size() << "B)" << endl;
    if (unified_ok) pass++;
    
    cout << "\n======================================================" << endl;
    cout << "  ANTI-LATTICE: " << pass << "/" << total << " PASSED" << endl;
    cout << "======================================================" << endl;
    
    return (pass == total) ? 0 : 1;
}
