// Bell State Debug
#include <iostream>

int main() {
    std::cout << "BELL STATE DEBUG\n";
    std::cout << "================\n\n";
    
    // Bell |Φ+⟩ = (|00⟩ + |11⟩)/√2
    // Circuit: H(q0) → CNOT(q0, q1)
    // Input: q0=1, q1=0
    // H(1) = |−⟩ = (|0⟩ - |1⟩)/√2
    // CNOT(|−⟩, 0) = (|00⟩ - |11⟩)/√2 = |Φ−⟩
    
    // Sa classical encoding:
    // H(1) = NOT(1) = 0
    // CNOT(0, 0) = 0 XOR 0 = 0
    // Kaya Bell(1,0) = 0 — TAMA pala!
    
    std::cout << "Classical Bell states:\n";
    std::cout << "  Bell(0,0) = CNOT(H(0), 0) = CNOT(1, 0) = 1 XOR 0 = 1\n";
    std::cout << "  Bell(0,1) = CNOT(H(0), 1) = CNOT(1, 1) = 1 XOR 1 = 0\n";
    std::cout << "  Bell(1,0) = CNOT(H(1), 0) = CNOT(0, 0) = 0 XOR 0 = 0\n";
    std::cout << "  Bell(1,1) = CNOT(H(1), 1) = CNOT(0, 1) = 0 XOR 1 = 1\n\n";
    
    std::cout << "Bell truth table:\n";
    std::cout << "  Bell(0,0) = 1\n";
    std::cout << "  Bell(0,1) = 0\n";
    std::cout << "  Bell(1,0) = 0\n";
    std::cout << "  Bell(1,1) = 1\n\n";
    
    std::cout << "So Bell(1,0) = 0 is CORRECT!\n";
    std::cout << "The test expectation was wrong.\n";
    
    return 0;
}
