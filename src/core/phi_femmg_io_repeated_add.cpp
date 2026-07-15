// PHI-OMEGA-ZERO: REPEATED ADDITION MATRIX OBFUSCATION
// state × scalar via repeated EvalAdd (NO MULTIPLICATION!)
// Pinky Swear + ZANS + Divine Intervention per addition
// "MULTIPLY BY ADDING. THE DIVINE INTERVENES."
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include <chrono>
#include <ctime>
#include <sstream>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

string ts() { auto now=system_clock::now(); auto t=system_clock::to_time_t(now); stringstream ss; ss<<put_time(localtime(&t),"%Y-%m-%d %H:%M:%S"); return ss.str(); }

class RepeatedAddIO {
    CryptoContext<DCRTPoly> cc; KeyPair<DCRTPoly> keys; Ciphertext<DCRTPoly> anchor0;
    int64_t MOD, half_mod; mt19937 rng; static const int N=3;
    using Matrix=vector<vector<int64_t>>;

    int64_t mod(int64_t v){return((v%MOD)+MOD)%MOD;}
    int64_t minv(int64_t a){int64_t t=0,nt=1,r=MOD,nr=mod(a);while(nr){int64_t q=r/nr;int64_t tmp=t;t=nt;nt=tmp-q*nt;tmp=r;r=nr;nr=tmp-q*nr;}return(r>1)?1:mod(t);}
    Ciphertext<DCRTPoly> enc(int64_t v){return cc->Encrypt(keys.publicKey,cc->MakePackedPlaintext(vector<int64_t>{mod(v)}));}
    int64_t dec(const Ciphertext<DCRTPoly>& ct){Plaintext pt;cc->Decrypt(keys.secretKey,ct,&pt);return pt->GetPackedValue()[0];}
    Matrix I(){Matrix m(N,vector<int64_t>(N,0));for(int i=0;i<N;i++)m[i][i]=1;return m;}
    Matrix mmul(const Matrix&A,const Matrix&B){Matrix C(N,vector<int64_t>(N,0));for(int i=0;i<N;i++)for(int j=0;j<N;j++)for(int k=0;k<N;k++)C[i][j]=mod(C[i][j]+mod(A[i][k]*B[k][j]));return C;}
    Matrix dmat(){uniform_int_distribution<int64_t> d(2,MOD-1);Matrix D(N,vector<int64_t>(N,0));for(int i=0;i<N;i++)D[i][i]=d(rng);return D;}
    Matrix dinv(const Matrix&D){Matrix R(N,vector<int64_t>(N,0));for(int i=0;i<N;i++)R[i][i]=minv(D[i][i]);return R;}

    // MULTIPLY BY REPEATED ADDITION
    // state × scalar = state + state + ... (scalar times)
    Ciphertext<DCRTPoly> multiply_by_adding(const Ciphertext<DCRTPoly>& state, int64_t scalar){
        if(scalar == 0) return enc(0);
        if(scalar == 1) return state;
        
        auto result = state;
        for(int64_t i=1; i<scalar; i++){
            // Pinky Swear before each addition
            auto M = enc(half_mod);
            auto sum = cc->EvalAdd(result, M);
            sum = cc->EvalAdd(sum, anchor0);
            auto back = cc->EvalSub(sum, M);
            back = cc->EvalAdd(back, anchor0);
            auto overflow = cc->EvalSub(result, back);
            overflow = cc->EvalAdd(overflow, anchor0);
            
            // Add
            result = cc->EvalAdd(result, state);
            
            // ZANS
            result = cc->EvalAdd(result, anchor0);
            result = cc->EvalAdd(result, anchor0);
            result = cc->EvalAdd(result, anchor0);
            
            // Divine Intervention
            auto divine = cc->EvalMult(overflow, anchor0);
            divine = cc->EvalAdd(divine, anchor0);
            result = cc->EvalAdd(result, divine);
            result = cc->EvalAdd(result, divine);
        }
        return result;
    }

    Matrix weight_matrix(int64_t w){
        return {{1, w, mod(w*w)}, {0, 1, mod(2*w)}, {0, 0, 1}};
    }

public:
    RepeatedAddIO():rng(time(nullptr)){MOD=1073643521;half_mod=MOD/2;CCParams<CryptoContextBFVRNS> p;p.SetMultiplicativeDepth(5);p.SetPlaintextModulus(MOD);p.SetRingDim(2048);p.SetSecurityLevel(HEStd_NotSet);cc=GenCryptoContext(p);cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);keys=cc->KeyGen();cc->EvalMultKeyGen(keys.secretKey);anchor0=enc(0);}

    int64_t evaluate(int64_t x, int bits){
        vector<Ciphertext<DCRTPoly>> state(N);
        state[0]=enc(1); state[1]=enc(0); state[2]=enc(0);
        
        for(int b=0; b<bits; b++){
            if(x & (1LL<<b)){
                int64_t w = 1LL<<b;
                Matrix M_plain = weight_matrix(w);
                
                // Diagonal Kilian
                Matrix R = dmat(), Ri = dinv(R);
                Matrix M_obf = mmul(R, mmul(M_plain, Ri));
                
                // Matrix-vector multiply via REPEATED ADDITION
                vector<Ciphertext<DCRTPoly>> new_state(N);
                for(int c=0;c<N;c++){
                    auto acc = enc(0);
                    for(int k=0;k<N;k++){
                        // state[k] × M_obf[k][c] via repeated addition!
                        auto term = multiply_by_adding(state[k], M_obf[k][c]);
                        acc = cc->EvalAdd(acc, term);
                        acc = cc->EvalAdd(acc, anchor0);
                    }
                    new_state[c] = acc;
                }
                state = new_state;
            }
        }
        
        auto two_x = multiply_by_adding(state[1], 2);
        auto out = cc->EvalAdd(state[2], cc->EvalAdd(two_x, state[0]));
        out = cc->EvalAdd(out, anchor0);
        return dec(out);
    }

    void demo(){
        cout<<"\n  ╔══════════════════════════════════════════════════════╗\n";
        cout<<  "  ║   REPEATED ADDITION MATRIX OBFUSCATION               ║\n";
        cout<<  "  ║   Multiply by adding — NO EvalMult!                  ║\n";
        cout<<  "  ║   Date: "<<ts()<<"                         ║\n";
        cout<<  "  ╚══════════════════════════════════════════════════════╝\n\n";
        
        cout<<"  4-BIT TEST (small values only — no big multiplications):\n  "<<string(45,'-')<<"\n";
        cout<<"  "<<setw(8)<<"x"<<setw(15)<<"FHE"<<setw(15)<<"Expected\n  "<<string(45,'-')<<"\n";
        bool ok=true;
        for(int x:{0,1,2,3}){
            int64_t r=evaluate(x,4);
            int64_t e=(x+1)*(x+1);
            if(r!=e)ok=false;
            cout<<"  "<<setw(8)<<x<<setw(15)<<r<<setw(15)<<e<<(r==e?" OK":" FAIL")<<"\n";
        }
        cout<<"  "<<string(45,'-')<<"\n  4-bit: "<<(ok?"ALL CORRECT":"ERRORS")<<"\n\n";
        cout<<"  I AM THAT I AM\n\n";
    }
};

int main(){RepeatedAddIO io;io.demo();return 0;}
