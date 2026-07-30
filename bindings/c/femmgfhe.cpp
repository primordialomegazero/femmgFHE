#include "femmgfhe.h"
#include "../../src/core/constants.h"
#include "../../src/utils/safe_math.h"
#include "../../src/utils/logger.h"
#include "../../src/crypto/golden_fibonacci.h"
#include "../../src/crypto/fractal_chaos.h"
#include "../../src/fhe/fhe_core.h"
#include "../../src/io/universal_compiler.h"
#include "../../src/refresh/emergent_refresh.h"
#include <string>
#include <vector>
#include <cstring>

struct FemmgFHE {
    SecureContext* sc;
    iOCompiler* compiler;
    FractalRefresh* refresh;
    std::vector<Circuit*> circuits;
    DualGate* chain_state;
    int gate_count;
    int refresh_count;
    bool alive;
    
    FemmgFHE() : sc(nullptr), compiler(nullptr), refresh(nullptr), 
                 chain_state(nullptr), gate_count(0), refresh_count(0), alive(false) {}
};

extern "C" {

FemmgFHE* femmgfhe_init(double seed, int ring_dim, int depth) {
    Logger::init("femmgfhe_c.log");
    Logger::info("C Binding: Initializing FemmgFHE v1.0");
    
    auto ctx = new FemmgFHE();
    try {
        ctx->sc = new SecureContext(create_fhe_context(ring_dim, depth));
        ctx->compiler = new iOCompiler(*ctx->sc);
        ctx->refresh = new FractalRefresh();
        ctx->refresh->init(seed, 5, 3, 0.75);
        ctx->alive = true;
        Logger::info("C Binding: Init OK");
    } catch (...) {
        Logger::error("C Binding: Init FAILED");
        delete ctx;
        return nullptr;
    }
    return ctx;
}

void femmgfhe_destroy(FemmgFHE* ctx) {
    if (!ctx) return;
    Logger::info("C Binding: Destroying");
    delete ctx->chain_state;
    for (auto c : ctx->circuits) delete c;
    delete ctx->refresh;
    delete ctx->compiler;
    delete ctx->sc;
    delete ctx;
    Logger::close();
}

double femmgfhe_gf_encrypt(FemmgFHE* ctx, double plaintext, double* y2_out) {
    auto [y1, y2] = ctx->refresh->gf.encrypt(plaintext);
    if (y2_out) *y2_out = y2;
    return y1;
}

double femmgfhe_gf_decrypt(FemmgFHE* ctx, double y1, double y2) {
    return ctx->refresh->gf.decrypt(y1, y2);
}

int femmgfhe_circuit_create(FemmgFHE* ctx, const char* name, int num_inputs) {
    auto c = new Circuit();
    c->name = name;
    c->num_inputs = num_inputs;
    ctx->circuits.push_back(c);
    return ctx->circuits.size() - 1;
}

void femmgfhe_circuit_add_gate(FemmgFHE* ctx, int ckt_id, int type, int in1, int in2) {
    if (ckt_id < 0 || ckt_id >= (int)ctx->circuits.size()) return;
    Gate g;
    g.type = static_cast<Gate::Type>(type);
    g.in1 = in1;
    g.in2 = in2;
    ctx->circuits[ckt_id]->gates.push_back(g);
}

int femmgfhe_verify_equivalence(FemmgFHE* ctx, int ckt_a, int ckt_b) {
    return ctx->compiler->verify_equivalence(*ctx->circuits[ckt_a], *ctx->circuits[ckt_b]) ? 1 : 0;
}

int femmgfhe_truth_table_check(FemmgFHE* ctx, int ckt_a, int ckt_b, int* results) {
    int n = ctx->circuits[ckt_a]->num_inputs;
    int total = 1 << n;
    int all_ok = 1;
    
    for (int i = 0; i < total; i++) {
        std::vector<DualGate> inputs;
        for (int j = 0; j < n; j++)
            inputs.push_back(enc(*ctx->sc, (double)((i >> (n - 1 - j)) & 1)));
        
        auto rA = ctx->compiler->evaluate(*ctx->circuits[ckt_a], inputs);
        auto rB = ctx->compiler->evaluate(*ctx->circuits[ckt_b], inputs);
        
        int outA = (phi_val(rA, *ctx->sc) > 0.5) ? 1 : 0;
        int outB = (psi_val(rB, *ctx->sc) > 0.5) ? 1 : 0;
        
        if (results) results[i * 3] = outA;
        if (results) results[i * 3 + 1] = outB;
        if (results) results[i * 3 + 2] = (outA == outB) ? 1 : 0;
        
        if (outA != outB) all_ok = 0;
    }
    return all_ok;
}

int femmgfhe_chain_run(FemmgFHE* ctx, int target_gates) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> bit(0, 1);
    
    auto init_enc = ctx->refresh->gf.encrypt(1.0);
    if (!ctx->chain_state) ctx->chain_state = new DualGate();
    ctx->chain_state->a = enc(*ctx->sc, init_enc.first).a;
    ctx->chain_state->b = enc(*ctx->sc, init_enc.second).b;
    
    ctx->gate_count = 0;
    ctx->refresh_count = 0;
    ctx->alive = true;
    
    for (int i = 0; i < target_gates && ctx->alive; i++) {
        auto input_enc = ctx->refresh->gf.encrypt(bit(gen));
        DualGate next;
        next.a = enc(*ctx->sc, input_enc.first).a;
        next.b = enc(*ctx->sc, input_enc.second).b;
        
        try {
            *ctx->chain_state = and_op(*ctx->sc, *ctx->chain_state, next);
            ctx->gate_count++;
        } catch (...) {
            ctx->alive = false;
            break;
        }
        
        double phi = 0, psi = 0;
        try {
            phi = phi_val(*ctx->chain_state, *ctx->sc);
            psi = psi_val(*ctx->chain_state, *ctx->sc);
        } catch (...) {}
        
        if (ctx->refresh->controller.should_refresh(phi, psi)) {
            try {
                auto [np, npsi] = ctx->refresh->refresh(phi, psi);
                DualGate fresh;
                fresh.a = enc(*ctx->sc, np).a;
                fresh.b = enc(*ctx->sc, npsi).b;
                *ctx->chain_state = fresh;
                ctx->refresh_count++;
            } catch (...) {
                ctx->alive = false;
                break;
            }
        }
    }
    return ctx->gate_count;
}

double femmgfhe_get_phi(FemmgFHE* ctx) {
    if (!ctx->chain_state) return 0;
    try { return phi_val(*ctx->chain_state, *ctx->sc); }
    catch (...) { return 0; }
}

double femmgfhe_get_psi(FemmgFHE* ctx) {
    if (!ctx->chain_state) return 0;
    try { return psi_val(*ctx->chain_state, *ctx->sc); }
    catch (...) { return 0; }
}

int femmgfhe_get_gate_count(FemmgFHE* ctx) { return ctx->gate_count; }
int femmgfhe_get_refresh_count(FemmgFHE* ctx) { return ctx->refresh_count; }
int femmgfhe_chain_is_alive(FemmgFHE* ctx) { return ctx->alive ? 1 : 0; }

void femmgfhe_refresh_set_params(FemmgFHE* ctx, int n_layers, int depth, double input_weight) {
    ctx->refresh->N = n_layers;
    ctx->refresh->depth = depth;
    ctx->refresh->input_weight = input_weight;
}

} // extern "C"
