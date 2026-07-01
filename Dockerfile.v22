FROM alpine:latest
LABEL org.opencontainers.image.title="FEmmg-FHE v22.1"
LABEL org.opencontainers.image.description="CTU v5 Triple Rashomon — 32B avalanche, 86K TPS (-O0), Quantum-resistant FHE"
LABEL org.opencontainers.image.version="22.1.0"
LABEL org.opencontainers.image.authors="Dan Joseph M. Fernandez / Primordial Omega Zero"

RUN apk add --no-cache g++ make libc-dev openssl-dev
WORKDIR /app
COPY . .

# Compile server
RUN g++ -std=c++17 -O3 -march=native -pthread \
    -I src/core -I src/chaos -I src/security -I src/kem -I src/storage -I src/math \
    -o femmg_server src/server/femmg_server.cpp -lm -lssl -lcrypto

# Compile benchmark
RUN g++ -std=c++17 -O0 -march=native -pthread \
    -I src/core -I src/chaos -I src/security -I src/kem -I src/storage -I src/math \
    -o test_ctu5_benchmark tests/test_ctu5_benchmark.cpp -lm -lssl -lcrypto

EXPOSE 8092
CMD ["./femmg_server"]
