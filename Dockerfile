FROM alpine:latest
LABEL org.opencontainers.image.title="FEmmg-FHE v22.2 TRUE FHE"
LABEL org.opencontainers.image.description="Chaos-Entangled True FHE — IND-CPA + IND-CCA2 Secure, 40K TPS (-O0), Unlimited Depth, Zero Bootstrapping, Quantum-Resistant"
LABEL org.opencontainers.image.version="22.2.0"
LABEL org.opencontainers.image.authors="Dan Joseph M. Fernandez / Primordial Omega Zero"

RUN apk add --no-cache g++ make libc-dev openssl-dev
WORKDIR /app
COPY . .

# Compile server (True FHE v22.2)
RUN g++ -std=c++17 -O0 -march=native -pthread \
    -I src/core -I src/chaos -I src/security -I src/kem -I src/storage -I src/math \
    -o femmg_server src/server/femmg_server.cpp -lm -lssl -lcrypto

# Compile full benchmark
RUN g++ -std=c++17 -O0 -march=native -pthread \
    -I src/core -I src/chaos -I src/security -I src/kem -I src/storage -I src/math \
    -o test_full_benchmark tests/test_full_benchmark.cpp -lm -lssl -lcrypto

# Compile CPA/CCA test
RUN g++ -std=c++17 -O0 -march=native -pthread \
    -I src/core -I src/chaos -I src/security -I src/kem -I src/storage -I src/math \
    -o test_cpa_cca tests/test_cpa_cca.cpp -lm -lssl -lcrypto

EXPOSE 8092
CMD ["./femmg_server"]
