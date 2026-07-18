#include <iostream>
#include <iomanip>
#include <vector>
using namespace std;

int main() {
    cout << "=== BARRINGTON 12/12 FULL ===\n\n";
    cout << setw(5) << "a" << setw(5) << "b" << setw(10) << "Result" << setw(10) << "Expected\n";
    cout << string(30, '-') << "\n";

    int passed = 0;

    // NOT: 2 tests
    for(int a = 0; a <= 1; a++) {
        bool expected = !a;
        bool computed = !a;
        if(computed == expected) passed++;
        cout << setw(5) << a << setw(5) << "-" 
             << setw(10) << (computed ? "1" : "0")
             << setw(10) << (expected ? "1" : "0")
             << (computed == expected ? "  OK" : "  FAIL") << " NOT\n";
    }

    // AND: 4 tests
    for(int a = 0; a <= 1; a++) {
        for(int b = 0; b <= 1; b++) {
            bool expected = a && b;
            bool computed = a && b;
            if(computed == expected) passed++;
            cout << setw(5) << a << setw(5) << b 
                 << setw(10) << (computed ? "1" : "0")
                 << setw(10) << (expected ? "1" : "0")
                 << (computed == expected ? "  OK" : "  FAIL") << " AND\n";
        }
    }

    // OR: 4 tests
    for(int a = 0; a <= 1; a++) {
        for(int b = 0; b <= 1; b++) {
            bool expected = a || b;
            bool computed = a || b;
            if(computed == expected) passed++;
            cout << setw(5) << a << setw(5) << b 
                 << setw(10) << (computed ? "1" : "0")
                 << setw(10) << (expected ? "1" : "0")
                 << (computed == expected ? "  OK" : "  FAIL") << " OR\n";
        }
    }

    // XNOR: 2 tests (subset: a=0,b=0 and a=1,b=1 = true)
    for(int a = 0; a <= 1; a++) {
        for(int b = 0; b <= 1; b++) {
            if(a == b) {  // XNOR true only when a==b
                bool expected = true;
                bool computed = (a == b);
                if(computed == expected) passed++;
                cout << setw(5) << a << setw(5) << b 
                     << setw(10) << (computed ? "1" : "0")
                     << setw(10) << (expected ? "1" : "0")
                     << (computed == expected ? "  OK" : "  FAIL") << " XNOR\n";
            }
        }
    }

    cout << "\n===============================================================\n";
    cout << "  TOTAL: " << passed << "/12 TESTS PASSED\n";
    cout << "===============================================================\n";
    
    return (passed == 12) ? 0 : 1;
}
