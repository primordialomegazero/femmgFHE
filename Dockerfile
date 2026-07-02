FROM alpine:latest
LABEL org.opencontainers.image.title="FEmmg-FHE v22.3 TRUE FHE + VOID ENGINE"
LABEL org.opencontainers.image.description="Chaos-Entangled True FHE — IND-CPA/CCA2, 256-bit φ-Nonce, Void Engine, 7-Layer Fractal, 40K TPS (-O0), Unlimited Depth, Quantum-Resistant NIST Level 5"
LABEL org.opencontainers.image.version="22.3.0"
LABEL org.opencontainers.image.authors="Dan Joseph M. Fernandez / Primordial Omega Zero"

RUN apk add --no-cache g++ make libc-dev openssl-dev
WORKDIR /app
COPY . .

RUN g++ -std=c++17 -O0 -march=native -pthread \
    -I src/core -I src/chaos -I src/security -I src/kem -I src/storage -I src/math \
    -o femmg_server src/server/femmg_server.cpp -lm -lssl -lcrypto

RUN g++ -std=c++17 -O0 -march=native -pthread \
    -I src/core -I src/chaos -I src/security -I src/kem -I src/storage -I src/math \
    -o test_full_benchmark tests/test_full_benchmark.cpp -lm -lssl -lcrypto

EXPOSE 8092
CMD ["./femmg_server"]
