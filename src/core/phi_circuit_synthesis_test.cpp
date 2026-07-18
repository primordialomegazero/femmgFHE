#include <iostream>
#include "phi_circuit_synthesis.h"

int main() {
    FullIOOrchestrator orchestrator(64);
    orchestrator.run_analysis();
    return 0;
}
