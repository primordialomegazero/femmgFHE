#include "../src/math/anti_lattice.h"
#include <iostream>
using namespace std;
using namespace anti_lattice;

int main() {
    CodingTheoryLayer coding;
    vector<uint8_t> msg = {1,0,1,0,1,1,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0};
    auto cw = coding.encode(msg);
    cout << "Original msg[0]=" << (int)msg[0] << " msg[1]=" << (int)msg[1] << endl;
    cout << "Codeword size: " << cw.size() << endl;
    
    coding.add_errors(cw, 3, 123);
    auto dec = coding.decode(cw, 3);
    cout << "Decoded msg[0]=" << (int)dec[0] << " msg[1]=" << (int)dec[1] << endl;
    cout << "Match: " << (dec[0] == msg[0] && dec[1] == msg[1] ? "YES" : "NO") << endl;
    
    return 0;
}
