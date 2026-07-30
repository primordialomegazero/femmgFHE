"""femmgfhe - Primordial Omega Zero iO Compiler Python Bindings"""
import ctypes
import os

_lib = ctypes.CDLL(os.path.join(os.path.dirname(__file__), 'libfemmgfhe.so'))

_lib.femmgfhe_init.argtypes = [ctypes.c_double, ctypes.c_int, ctypes.c_int]
_lib.femmgfhe_init.restype = ctypes.c_void_p

_lib.femmgfhe_destroy.argtypes = [ctypes.c_void_p]
_lib.femmgfhe_gf_encrypt.argtypes = [ctypes.c_void_p, ctypes.c_double, ctypes.POINTER(ctypes.c_double)]
_lib.femmgfhe_gf_encrypt.restype = ctypes.c_double
_lib.femmgfhe_gf_decrypt.argtypes = [ctypes.c_void_p, ctypes.c_double, ctypes.c_double]
_lib.femmgfhe_gf_decrypt.restype = ctypes.c_double

_lib.femmgfhe_circuit_create.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_int]
_lib.femmgfhe_circuit_create.restype = ctypes.c_int
_lib.femmgfhe_circuit_add_gate.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int]
_lib.femmgfhe_verify_equivalence.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_int]
_lib.femmgfhe_verify_equivalence.restype = ctypes.c_int

_lib.femmgfhe_chain_run.argtypes = [ctypes.c_void_p, ctypes.c_int]
_lib.femmgfhe_chain_run.restype = ctypes.c_int
_lib.femmgfhe_get_phi.argtypes = [ctypes.c_void_p]
_lib.femmgfhe_get_phi.restype = ctypes.c_double
_lib.femmgfhe_get_psi.argtypes = [ctypes.c_void_p]
_lib.femmgfhe_get_psi.restype = ctypes.c_double
_lib.femmgfhe_get_gate_count.argtypes = [ctypes.c_void_p]
_lib.femmgfhe_get_gate_count.restype = ctypes.c_int
_lib.femmgfhe_get_refresh_count.argtypes = [ctypes.c_void_p]
_lib.femmgfhe_get_refresh_count.restype = ctypes.c_int
_lib.femmgfhe_chain_is_alive.argtypes = [ctypes.c_void_p]
_lib.femmgfhe_chain_is_alive.restype = ctypes.c_int

AND, OR, NAND, NOR, XOR, NOT = 0, 1, 2, 3, 4, 5

class FemmgFHE:
    def __init__(self, seed=0.123456789, ring_dim=4096, depth=120):
        self.ctx = _lib.femmgfhe_init(seed, ring_dim, depth)
        if not self.ctx:
            raise RuntimeError("Failed to initialize FemmgFHE")
    
    def __del__(self):
        if self.ctx:
            _lib.femmgfhe_destroy(self.ctx)
    
    def gf_encrypt(self, plaintext):
        y2 = ctypes.c_double()
        y1 = _lib.femmgfhe_gf_encrypt(self.ctx, plaintext, ctypes.byref(y2))
        return (y1, y2.value)
    
    def gf_decrypt(self, y1, y2):
        return _lib.femmgfhe_gf_decrypt(self.ctx, y1, y2)
    
    def create_circuit(self, name, num_inputs):
        return _lib.femmgfhe_circuit_create(self.ctx, name.encode(), num_inputs)
    
    def add_gate(self, ckt_id, gate_type, in1, in2):
        _lib.femmgfhe_circuit_add_gate(self.ctx, ckt_id, gate_type, in1, in2)
    
    def verify_equivalence(self, ckt_a, ckt_b):
        return bool(_lib.femmgfhe_verify_equivalence(self.ctx, ckt_a, ckt_b))
    
    def run_chain(self, target=1000):
        gates = _lib.femmgfhe_chain_run(self.ctx, target)
        return {
            'gates': gates,
            'refreshes': self.refresh_count,
            'phi': self.phi,
            'psi': self.psi,
            'alive': self.alive
        }
    
    @property
    def phi(self): return _lib.femmgfhe_get_phi(self.ctx)
    @property
    def psi(self): return _lib.femmgfhe_get_psi(self.ctx)
    @property
    def gate_count(self): return _lib.femmgfhe_get_gate_count(self.ctx)
    @property
    def refresh_count(self): return _lib.femmgfhe_get_refresh_count(self.ctx)
    @property
    def alive(self): return bool(_lib.femmgfhe_chain_is_alive(self.ctx))


# Demo
if __name__ == '__main__':
    print("╔══════════════════════════════════════════════╗")
    print("║  FEMMGFHE Python Bindings v1.0              ║")
    print("║  Primordial Omega Zero iO                   ║")
    print("╚══════════════════════════════════════════════╝")
    
    ctx = FemmgFHE(seed=0.123456789)
    print(f"Golden Fibonacci: 0.75 → {ctx.gf_encrypt(0.75)}")
    
    cA = ctx.create_circuit("Circuit A: (X AND Y) OR Z", 3)
    ctx.add_gate(cA, AND, 0, 1)
    ctx.add_gate(cA, OR, 3, 2)
    
    cB = ctx.create_circuit("Circuit B: (X OR Z) AND (Y OR Z)", 3)
    ctx.add_gate(cB, OR, 0, 2)
    ctx.add_gate(cB, OR, 1, 2)
    ctx.add_gate(cB, AND, 3, 4)
    
    print(f"Equivalence: {ctx.verify_equivalence(cA, cB)}")
    
    print("Running chain...")
    result = ctx.run_chain(100)
    print(f"Gates: {result['gates']}, Refreshes: {result['refreshes']}")
    print(f"φ={result['phi']:.4f}, ψ={result['psi']:.4f}")
