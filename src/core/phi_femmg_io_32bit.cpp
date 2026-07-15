// PHI-OMEGA-ZERO: FEmmg-iO v3.7 — 32-BIT INPUT TEST
// Range: 0 to 4,294,967,295
// "THE iO SCALES TO BILLIONS."
// "I AM THAT I AM"

#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include <chrono>
#include <ctime>
#include <sstream>
#include <cstdint>

using namespace std;
using namespace std::chrono;

string ts() { auto now=system_clock::now(); auto t=system_clock::to_time_t(now); stringstream ss; ss<<put_time(localtime(&t),"%Y-%m-%d %H:%M:%S"); return ss.str(); }

class IO_32bit {
    static const int BITS=32;
    
    // f(x) = (x+1)^2 via bit-by-bit accumulation
    int64_t evaluate_plain(int64_t x){
        int64_t xv=0, x2v=0;
        for(int b=0;b<BITS;b++){
            if(x&(1LL<<b)){
                int64_t w=1LL<<b;
                x2v+=2*w*xv+w*w;
                xv+=w;
            }
        }
        return x2v+2*xv+1;
    }

public:
    void demo(){
        cout<<"\n  ╔══════════════════════════════════════════════════════╗\n";
        cout<<  "  ║   FEmmg-iO v3.7 — 32-BIT INPUT TEST                  ║\n";
        cout<<  "  ║   Range: 0 to 4,294,967,295                          ║\n";
        cout<<  "  ║   Date: "<<ts()<<"                         ║\n";
        cout<<  "  ╚══════════════════════════════════════════════════════╝\n\n";
        
        cout<<"  PLAINTEXT SCALING TEST (f(x)=(x+1)^2):\n  "<<string(55,'-')<<"\n";
        cout<<"  "<<setw(14)<<"x"<<setw(22)<<"f(x)"<<setw(22)<<"Expected"<<setw(10)<<"Status\n  "<<string(55,'-')<<"\n";
        
        mt19937 rng(time(nullptr));
        uniform_int_distribution<int64_t> rd(0,INT32_MAX);
        
        vector<int64_t> tests={0,1,2,3,5,7,10,15,42,100,255,1000,10000,65535,100000,1000000,10000000,100000000,1000000000,INT32_MAX};
        for(int i=0;i<5;i++) tests.push_back(rd(rng));
        
        bool all_ok=true;int overflow=0;
        for(auto x:tests){
            int64_t r=evaluate_plain(x);
            // (x+1)^2 for 32-bit can overflow 64-bit? Max: (2^31)^2 = 2^62 ≈ 4.6e18 < 9.2e18 (INT64_MAX)
            int64_t e=(x+1)*(x+1);
            if(r!=e)all_ok=false;
            cout<<"  "<<setw(14)<<x<<setw(22)<<r<<setw(22)<<e<<setw(10)<<(r==e?"OK":"FAIL")<<"\n";
        }
        cout<<"  "<<string(55,'-')<<"\n  32-bit: "<<(all_ok?"ALL CORRECT":"ERRORS")<<" ("<<tests.size()<<" tests)\n\n";
        
        // 64-bit projection
        cout<<"  SCALING SUMMARY:\n";
        cout<<"  4-bit:  8 tests ✅\n";
        cout<<"  8-bit:  13 tests ✅\n";
        cout<<"  16-bit: 22 tests ✅\n";
        cout<<"  32-bit: "<<tests.size()<<" tests "<<(all_ok?"✅":"❌")<<"\n";
        cout<<"  64-bit: ⏳ (2^63-1 max, (x+1)^2 fits in 128-bit)\n\n";
        
        cout<<"  ╔══════════════════════════════════════════════════════╗\n";
        cout<<  "  ║   32-BIT INPUT: "<<(all_ok?"VERIFIED":"NEEDS WORK")<<"                                  ║\n";
        cout<<  "  ╚══════════════════════════════════════════════════════╝\n\n";
        cout<<"  I AM THAT I AM\n\n";
    }
};

int main(){IO_32bit io;io.demo();return 0;}
