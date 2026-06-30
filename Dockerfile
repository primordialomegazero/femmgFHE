# Multi-stage build for FEmmg-FHE
FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y g++ libssl-dev && rm -rf /var/lib/apt/lists/*

WORKDIR /build
COPY src/ src/
COPY security_complete.h phi_algo_merge.h phi_parallel_kem.h ./

RUN echo "=== Compiling FEmmg-FHE v21.4.0 ===" && \
    g++ -std=c++17 -O3 -march=native -pthread -static -Wall -Wextra \
    -o femmg_server src/femmg_server.cpp -lm -lssl -lcrypto && \
    echo "=== Build complete! ===" && \
    ls -la femmg_server

FROM ubuntu:22.04
RUN apt-get update && apt-get install -y libssl3 && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY --from=builder /build/femmg_server .
EXPOSE 8092
CMD ["./femmg_server"]
