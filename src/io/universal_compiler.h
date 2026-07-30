#pragma once
#include "../fhe/fhe_core.h"
#include "../utils/logger.h"
#include <vector>
#include <string>
#include <stdexcept>

struct Gate {
    enum Type { AND, OR, NAND, NOR, XOR, NOT };
    Type type;
    int in1, in2;
};

struct Circuit {
    int num_inputs;
    std::vector<Gate> gates;
    std::string name;
};

struct iOCompiler {
    SecureContext& sc;
    iOCompiler(SecureContext& ctx) : sc(ctx) {}
    
    DualGate evaluate_gate(Gate& g, std::vector<DualGate>& wires) {
        DualGate& w1 = wires[g.in1];
        switch(g.type) {
            case Gate::AND: { DualGate& w2=wires[g.in2]; return and_op(sc,w1,w2); }
            case Gate::OR:  { DualGate& w2=wires[g.in2]; return or_op(sc,w1,w2); }
            case Gate::NAND:{ DualGate& w2=wires[g.in2]; return nand_op(sc,w1,w2); }
            case Gate::NOR: { DualGate w2=wires[g.in2]; auto o=or_op(sc,w1,w2); return not_op(sc,o); }
            case Gate::XOR: { DualGate& w2=wires[g.in2]; return xor_op(sc,w1,w2); }
            case Gate::NOT: return not_op(sc,w1);
            default: throw std::runtime_error("Unknown gate");
        }
    }
    
    DualGate evaluate(Circuit& c, std::vector<DualGate>& inputs) {
        std::vector<DualGate> wires = inputs;
        for(auto& g : c.gates) {
            try { wires.push_back(evaluate_gate(g, wires)); }
            catch(const std::exception& e) { Logger::error("Gate eval failed: "+std::string(e.what())); throw; }
        }
        if(wires.empty()) throw std::runtime_error("No output");
        return wires.back();
    }
    
    DualGate iO_encode(DualGate& rA, DualGate& rB) {
        auto phi_pt = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{PHI});
        auto inv_denom = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0/(PHI-PSI)});
        auto diff_a = sc.cc->EvalSub(rA.a, rB.a), diff_b = sc.cc->EvalSub(rA.b, rB.b);
        DualGate out;
        out.b = sc.cc->EvalAdd(sc.cc->EvalMult(diff_a, inv_denom), sc.cc->EvalMult(diff_b, inv_denom));
        out.a = sc.cc->EvalSub(rA.a, sc.cc->EvalMult(out.b, phi_pt));
        return out;
    }
    
    bool verify_equivalence(Circuit& cA, Circuit& cB) {
        int n = cA.num_inputs, total = 1 << n;
        for(int i=0;i<total;i++){
            std::vector<DualGate> inputs;
            for(int j=0;j<n;j++) inputs.push_back(enc(sc, (i>>(n-1-j))&1));
            DualGate rA=evaluate(cA,inputs), rB=evaluate(cB,inputs);
            int outA=(phi_val(rA,sc)>0.5)?1:0, outB=(psi_val(rB,sc)>0.5)?1:0;
            if(outA!=outB){ Logger::error("Equiv fail at "+std::to_string(i)); return false; }
        }
        return true;
    }
};
