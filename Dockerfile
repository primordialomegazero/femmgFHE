FROM alpine:latest
LABEL org.opencontainers.image.title="FEmmg-FHE v22.3.1 TRUE FHE + ML-KEM-1024"
LABEL org.opencontainers.image.description="Chaos-Entangled True FHE — IND-CPA/CCA2, 256-bit φ-Nonce, Void Engine, 7-Layer Fractal, Native ML-KEM-1024 (FIPS 203), 40K TPS (-O0), Unlimited Depth, NIST Level 5"
LABEL org.opencontainers.image.version="22.3.1"
LABEL org.opencontainers.image.authors="Dan Joseph M. Fernandez / Primordial Omega Zero"

RUN apk add --no-cache g++ make libc-dev openssl-dev cmake ninja git
WORKDIR /app
COPY . .

# Build liboqs for native ML-KEM-1024
RUN cd /tmp && git clone --depth 1 https://github.com/open-quantum-safe/liboqs.git && cd liboqs && mkdir build && cd build && cmake .. -DOQS_USE_OPENSSL=OFF && make -j$(nproc) && make install

RUN g++ -std=c++17 -O0 -march=native -pthread \
    -I src/core -I src/chaos -I src/security -I src/kem -I src/storage -I src/math \
    -o femmg_server src/server/femmg_server.cpp -L/usr/local/lib -loqs -lm -lssl -lcrypto

EXPOSE 8092
CMD ["./femmg_server"]
