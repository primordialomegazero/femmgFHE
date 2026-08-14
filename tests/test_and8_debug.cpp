#include <iostream>
#include <vector>

int main() {
    std::cout << "8-input AND Debug\n\n";
    
    // Current circuit:
    // n01 = NAND(0,1), n23 = NAND(2,3), n45 = NAND(4,5), n67 = NAND(6,7)
    // a01 = NAND(n01,n01), a23 = NAND(n23,n23), a45 = NAND(n45,n45), a67 = NAND(n67,n67)
    // m0123 = NAND(a01,a23), m4567 = NAND(a45,a67)
    // m_all = NAND(m0123,m4567)
    // and_result = NAND(m_all,m_all)
    
    // Test case: {1,1,1,1,1,1,1,1} (dapat AND=1)
    std::vector<bool> in = {1,1,1,1,1,1,1,1};
    
    bool n01 = !(in[0] && in[1]);  // NAND(1,1) = 0
    bool n23 = !(in[2] && in[3]);  // NAND(1,1) = 0
    bool n45 = !(in[4] && in[5]);  // NAND(1,1) = 0
    bool n67 = !(in[6] && in[7]);  // NAND(1,1) = 0
    
    bool a01 = !(n01 && n01);  // NAND(0,0) = 1
    bool a23 = !(n23 && n23);  // NAND(0,0) = 1
    bool a45 = !(n45 && n45);  // NAND(0,0) = 1
    bool a67 = !(n67 && n67);  // NAND(0,0) = 1
    
    bool m0123 = !(a01 && a23);  // NAND(1,1) = 0
    bool m4567 = !(a45 && a67);  // NAND(1,1) = 0
    
    bool m_all = !(m0123 && m4567);  // NAND(0,0) = 1
    bool and_result = !(m_all && m_all);  // NAND(1,1) = 0
    
    std::cout << "Test (all 1s):\n";
    std::cout << "  n01..n67 = " << n01 << n23 << n45 << n67 << "\n";
    std::cout << "  a01..a67 = " << a01 << a23 << a45 << a67 << "\n";
    std::cout << "  m0123 = " << m0123 << ", m4567 = " << m4567 << "\n";
    std::cout << "  m_all = " << m_all << ", and_result = " << and_result << "\n";
    std::cout << "  Expected AND = 1, Got = " << and_result << " ❌\n\n";
    
    // Ang problema: ang circuit ay hindi 8-input AND
    // Ito ay tree ng NAND na nagri-result sa XOR-like behavior
    
    // TAMANG 8-input AND:
    // AND(all) = NOT(OR(any NOT(input)))
    // Mas simple: sequential AND
    
    bool result = in[0];
    for (int i = 1; i < 8; i++) {
        bool nand_ab = !(result && in[i]);
        result = !(nand_ab && nand_ab);  // NOT(NAND) = AND
    }
    
    std::cout << "Sequential AND: " << result << " (expected 1)\n";
    
    return 0;
}
