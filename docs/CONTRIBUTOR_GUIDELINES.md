# FEmmg-FHE v6.0 — Community Contributor Guidelines

## Setup
git clone https://github.com/openfheorg/openfhe-development.git
cd openfhe-development && mkdir build && cd build
cmake .. && make -j$(nproc) && sudo make install
git clone https://github.com/primordialomegazero/femmgFHE.git
cd femmgFHE && make all && ./tests/full_blown_test.sh

## Priority Areas
### HIGH
- Formal RLWE Security Proofs (mathematicians needed)
- Ring Dim 32768 Benchmarks (cloud/server)
- Cross-Library Validation (SEAL, HElib, TFHE)
- NIST PQ-KEM Security Analysis

### MEDIUM
- GPU Acceleration (CUDA/OpenCL)
- Python Bindings (PyFEmmg)
- Documentation & Tutorials
- Docker Deployment

### NICE TO HAVE
- Web Interface Demo
- Benchmark Dashboard
- Academic Peer Review

## Contribution Process
1. Fork repo
2. Feature branch: feature/your-feature
3. Write tests (36/36 must pass)
4. Submit PR with: problem solved, test method, performance impact

## Code Standards
- phi_ prefix for core modules
- 4-space indentation
- No hardcoding
- Every module has test
