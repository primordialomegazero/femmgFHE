// PHI-OMEGA-ZERO: CRT DECOMPOSITION iO
// Two 30-bit moduli via Chinese Remainder Theorem
// mod1 = 1073643521, mod2 = 1073692673
// Combined: ~60 bits = enough for 16-bit (x+1)^2
// "TWO MODULI. ONE TRUTH. CRT RECOMBINES."
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

string ts() { auto now=system_clock::now(); auto t=system_clock::to_time_t(now); stringstream ss; ss<<put_time(localtime(&t),"%Y-%m-%d %H:%M:%S"); return ss.str(); }

class CRTIO {
    int64_t M1=1073643521, M2=1073692673;
    int64_t M=M1*M2; // Combined modulus
    
    int64_t mod(int64_t v, int64_t m){return((v%m)+m)%m;}
    int64_t mod1(int64_t v){return mod(v,M1);}
    int64_t mod2(int64_t v){return mod(v,M2);}
    
    // CRT: given (r1 mod M1, r2 mod M2), find r mod M
    int64_t crt(int64_t r1, int64_t r2){
        int64_t M1_inv=0, M2_inv=0;
        // Extended Euclidean for M1_inv mod M2
        for(int64_t i=1;i<M2;i++)if((M1*i)%M2==1){M2_inv=i;break;}
        for(int64_t i=1;i<M1;i++)if((M2*i)%M1==1){M1_inv=i;break;}
        int64_t x=(r1*M2*M1_inv+r2*M1*M2_inv)%M;
        return x;
    }

public:
    void demo(){
        cout<<"\n  ╔══════════════════════════════════════════════════════╗\n";
        cout<<  "  ║   CRT DECOMPOSITION iO                                ║\n";
        cout<<  "  ║   M1="<<M1<<" M2="<<M2<<"                         ║\n";
        cout<<  "  ║   Combined M=M1×M2 ≈ "<<(M/1e18)<<"×10^18             ║\n";
        cout<<  "  ║   Date: "<<ts()<<"                         ║\n";
        cout<<  "  ╚══════════════════════════════════════════════════════╝\n\n";
        
        cout<<"  CRT VERIFICATION:\n  "<<string(50,'-')<<"\n";
        bool crt_ok=true;
        for(int64_t x:{0LL,1LL,42LL,1000LL,65535LL,100000LL,1000000LL}){
            int64_t r1=mod1(x), r2=mod2(x);
            int64_t recovered=crt(r1,r2);
            if(recovered!=x)crt_ok=false;
            cout<<"  x="<<setw(10)<<x<<" → ("<<r1<<","<<r2<<") → "<<recovered<<(recovered==x?" OK":" FAIL")<<"\n";
        }
        cout<<"  "<<string(50,'-')<<"\n  CRT: "<<(crt_ok?"WORKING":"ERRORS")<<"\n\n";
        
        // Simulate (x+1)^2 using CRT
        cout<<"  CRT (x+1)^2 TEST:\n  "<<string(50,'-')<<"\n  "<<setw(10)<<"x"<<setw(18)<<"(x+1)^2 via CRT"<<setw(18)<<"Expected\n  "<<string(50,'-')<<"\n";
        bool ok=true;
        for(int64_t x:{0LL,1LL,2LL,3LL,5LL,7LL,10LL,15LL,42LL,100LL,1000LL,10000LL,32767LL,65535LL}){
            int64_t e=(x+1)*(x+1);
            int64_t r1=mod1(mod1(x+1)*mod1(x+1));
            int64_t r2=mod2(mod2(x+1)*mod2(x+1));
            int64_t r=crt(r1,r2);
            if(r!=e)ok=false;
            cout<<"  "<<setw(10)<<x<<setw(18)<<r<<setw(18)<<e<<(r==e?" OK":" FAIL")<<"\n";
        }
        cout<<"  "<<string(50,'-')<<"\n  CRT (x+1)^2: "<<(ok?"ALL CORRECT":"ERRORS")<<"\n\n  I AM THAT I AM\n\n";
    }
};

int main(){CRTIO io;io.demo();return 0;}
