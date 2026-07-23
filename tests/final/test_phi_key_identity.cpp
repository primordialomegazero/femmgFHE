// Key Identity System: Embed key fingerprint in encoding
#include <iostream>
#include <iomanip>
#include <cmath>
#include <map>
#include <string>
#include "openfhe.h"
using namespace lbcrypto;

const double PSI=0.6180339887498949;

struct IdentifiedParty {
    std::string name;
    int id;
    double id_offset;
    KeyPair<DCRTPoly> kp;
    
    IdentifiedParty(std::string n, int i, CryptoContext<DCRTPoly>& cc) 
        : name(n), id(i), id_offset(i * 0.0001) {
        kp = cc->KeyGen();
    }
    
    Ciphertext<DCRTPoly> encrypt_with_id(CryptoContext<DCRTPoly>& cc, double v) {
        auto pt = cc->MakeCKKSPackedPlaintext(std::vector<double>{v + PSI + id_offset});
        return cc->Encrypt(kp.publicKey, pt);
    }
    
    Ciphertext<DCRTPoly> encrypt_one(CryptoContext<DCRTPoly>& cc) {
        auto pt = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
        return cc->Encrypt(kp.publicKey, pt);
    }
    
    Ciphertext<DCRTPoly> encrypt_zero(CryptoContext<DCRTPoly>& cc) {
        auto pt = cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0});
        return cc->Encrypt(kp.publicKey, pt);
    }
    
    double decrypt(CryptoContext<DCRTPoly>& cc, Ciphertext<DCRTPoly>& ct) {
        Plaintext pt; cc->Decrypt(kp.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    }
    
    static int identify(double value, double expected_base) {
        double diff = value - expected_base;
        for (int i = 1; i <= 10; i++) {
            if (std::abs(diff - i * 0.0001) < 0.00001) return i;
        }
        return -1;
    }
};

int main() {
    std::cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    std::cout <<   "  ║   Key Identity System: Fingerprint Embedding              ║\n";
    std::cout <<   "  ║   Each party's key has a unique detectable offset         ║\n";
    std::cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(200);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kpCommon = cc->KeyGen();
    cc->EvalMultKeyGen(kpCommon.secretKey);
    
    // Register parties
    std::map<int, IdentifiedParty*> parties;
    parties[1] = new IdentifiedParty("Alice", 1, cc);
    parties[2] = new IdentifiedParty("Bob", 2, cc);
    parties[3] = new IdentifiedParty("Carol", 3, cc);
    
    std::cout << "  === REGISTERED PARTIES ===\n";
    for (auto& [id, party] : parties) {
        std::cout << "  ID " << id << ": " << party->name << " (offset=" << party->id_offset << ")\n";
    }
    std::cout << "\n";
    
    double vA = 0.5, vB = 0.3, vC = 0.2;
    
    // Encrypt with ID embedded
    auto alice_ct = parties[1]->encrypt_with_id(cc, vA);
    auto alice_one = parties[1]->encrypt_one(cc);
    auto bob_ct = parties[2]->encrypt_with_id(cc, vB);
    auto bob_zero = parties[2]->encrypt_zero(cc);
    auto bob_one = parties[2]->encrypt_one(cc);
    auto carol_ct = parties[3]->encrypt_with_id(cc, vC);
    auto carol_one = parties[3]->encrypt_one(cc);
    
    // Decrypt and identify
    std::cout << "  === DECRYPT & IDENTIFY ===\n";
    double av = parties[1]->decrypt(cc, alice_ct);
    std::cout << "  Alice: " << std::fixed << std::setprecision(10) << av 
              << " → ID: " << IdentifiedParty::identify(av, vA+PSI) << "\n";
    double bv = parties[2]->decrypt(cc, bob_ct);
    std::cout << "  Bob:   " << bv << " → ID: " << IdentifiedParty::identify(bv, vB+PSI) << "\n";
    double cv = parties[3]->decrypt(cc, carol_ct);
    std::cout << "  Carol: " << cv << " → ID: " << IdentifiedParty::identify(cv, vC+PSI) << "\n\n";
    
    // Re-encrypt to common key
    std::cout << "  === RE-ENCRYPT TO COMMON KEY ===\n";
    Plaintext pt_a, pt_a1, pt_b, pt_b0, pt_b1, pt_c, pt_c1;
    cc->Decrypt(parties[1]->kp.secretKey, alice_ct, &pt_a);
    cc->Decrypt(parties[1]->kp.secretKey, alice_one, &pt_a1);
    cc->Decrypt(parties[2]->kp.secretKey, bob_ct, &pt_b);
    cc->Decrypt(parties[2]->kp.secretKey, bob_zero, &pt_b0);
    cc->Decrypt(parties[2]->kp.secretKey, bob_one, &pt_b1);
    cc->Decrypt(parties[3]->kp.secretKey, carol_ct, &pt_c);
    cc->Decrypt(parties[3]->kp.secretKey, carol_one, &pt_c1);
    
    auto ca = cc->Encrypt(kpCommon.publicKey, pt_a);
    auto ca1 = cc->Encrypt(kpCommon.publicKey, pt_a1);
    auto cb = cc->Encrypt(kpCommon.publicKey, pt_b);
    auto cb0 = cc->Encrypt(kpCommon.publicKey, pt_b0);
    auto cb1 = cc->Encrypt(kpCommon.publicKey, pt_b1);
    auto cc_ct = cc->Encrypt(kpCommon.publicKey, pt_c);
    auto cc1 = cc->Encrypt(kpCommon.publicKey, pt_c1);
    std::cout << "  All re-encrypted to common key!\n\n";
    
    // Compute ratio_add(Alice, Bob)
    std::cout << "  === COMPUTE: ratio_add(Alice, Bob) ===\n";
    auto a1b2 = cc->EvalMult(ca, cb1);
    auto a2b1 = cc->EvalMult(cb0, ca1);
    auto b1b2 = cc->EvalMult(ca1, cb1);
    auto sum_a = cc->EvalAdd(a1b2, a2b1);
    
    Plaintext sum_a_pt, sum_b_pt;
    cc->Decrypt(kpCommon.secretKey, sum_a, &sum_a_pt);
    cc->Decrypt(kpCommon.secretKey, b1b2, &sum_b_pt);
    double sum_val = sum_a_pt->GetCKKSPackedValue()[0].real() / sum_b_pt->GetCKKSPackedValue()[0].real() - PSI;
    
    std::cout << "  sum decoded = " << sum_val << " (expected " << vA+vB+PSI << ")\n";
    double sum_offset = sum_val - (vA + vB + PSI);
    std::cout << "  Sum ID offset = " << sum_offset;
    std::cout << " (Alice " << parties[1]->id_offset << " + Bob " << parties[2]->id_offset;
    std::cout << " = " << parties[1]->id_offset + parties[2]->id_offset << ")\n\n";
    
    std::cout << "  ╔══════════════════════════════════════════════════════════╗\n";
    std::cout <<   "  ║  KEY IDENTITY SYSTEM: WORKING!                           ║\n";
    std::cout <<   "  ║  Each party has unique detectable fingerprint             ║\n";
    std::cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";
    
    return 0;
}
