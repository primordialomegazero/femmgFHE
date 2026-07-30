#ifndef FEMMGFHE_H
#define FEMMGFHE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FemmgFHE FemmgFHE;

// Lifecycle
FemmgFHE* femmgfhe_init(double seed, int ring_dim, int depth);
void femmgfhe_destroy(FemmgFHE* ctx);

// Golden Fibonacci
double femmgfhe_gf_encrypt(FemmgFHE* ctx, double plaintext, double* y2_out);
double femmgfhe_gf_decrypt(FemmgFHE* ctx, double y1, double y2);

// Universal Compiler
int femmgfhe_circuit_create(FemmgFHE* ctx, const char* name, int num_inputs);
void femmgfhe_circuit_add_gate(FemmgFHE* ctx, int ckt_id, int type, int in1, int in2);
int femmgfhe_verify_equivalence(FemmgFHE* ctx, int ckt_a, int ckt_b);
int femmgfhe_truth_table_check(FemmgFHE* ctx, int ckt_a, int ckt_b, int* results);

// iO Chain
int femmgfhe_chain_run(FemmgFHE* ctx, int target_gates);
double femmgfhe_get_phi(FemmgFHE* ctx);
double femmgfhe_get_psi(FemmgFHE* ctx);
int femmgfhe_get_gate_count(FemmgFHE* ctx);
int femmgfhe_get_refresh_count(FemmgFHE* ctx);
int femmgfhe_chain_is_alive(FemmgFHE* ctx);

// Fractal Refresh
void femmgfhe_refresh_set_params(FemmgFHE* ctx, int n_layers, int depth, double input_weight);
int femmgfhe_refresh_manual(FemmgFHE* ctx);
int femmgfhe_refresh_get_auto(FemmgFHE* ctx);

#ifdef __cplusplus
}
#endif
#endif
