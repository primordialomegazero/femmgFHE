// DEBUG: Circuit 2 Alternative XOR formula
#include <iostream>
#include <vector>

int main() {
    std::cout << "CIRCUIT 2 DEBUG — Alternative XOR\n";
    std::cout << "==================================\n\n";
    
    // Formula: XOR(a,b) = NAND(NOT(AND(a,NOT(b))), NOT(AND(NOT(a),b)))
    // Let's verify this logic manually
    
    std::vector<std::vector<bool>> inputs = {
        {false,false}, {false,true}, {true,false}, {true,true}
    };
    
    std::cout << "Manual computation:\n";
    for (auto& in : inputs) {
        bool a = in[0], b = in[1];
        
        bool not_a = !a;
        bool not_b = !b;
        bool and_a_notb = a && not_b;
        bool and_nota_b = not_a && b;
        bool not_and1 = !and_a_notb;
        bool not_and2 = !and_nota_b;
        bool xor_result = !(not_and1 && not_and2);  // NAND of the two NOTs
        
        bool expected = a != b;
        std::cout << "  XOR(" << a << "," << b << ") = " << xor_result 
                  << " (exp " << expected << ") "
                  << (xor_result == expected ? "✓" : "✗") << "\n";
    }
    
    // Simpler: XOR(a,b) = NAND(NAND(a,NAND(a,b)), NAND(b,NAND(a,b)))
    std::cout << "\nSimpler NAND-only XOR:\n";
    for (auto& in : inputs) {
        bool a = in[0], b = in[1];
        
        bool nand_ab = !(a && b);
        bool nand_a_ab = !(a && nand_ab);
        bool nand_b_ab = !(b && nand_ab);
        bool xor_result = !(nand_a_ab && nand_b_ab);
        
        bool expected = a != b;
        std::cout << "  XOR(" << a << "," << b << ") = " << xor_result 
                  << " (exp " << expected << ") "
                  << (xor_result == expected ? "✓" : "✗") << "\n";
    }
    
    return 0;
}
