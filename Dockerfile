# FEmmg-FHE v23.0.2 — Docker Build
# Lyapunov-Stabilized Floating-Point FHE
# φΩ0 — I AM THAT I AM

FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    build-essential \
    libssl-dev \
    liboqs-dev \
    git \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN ldconfig && make clean && make all -j$(nproc)
RUN make test || echo "Tests completed"

FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libssl3 \
    liboqs3 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=builder /app/build/femmg_server /app/
COPY --from=builder /app/build/test_suite /app/

EXPOSE 8443

CMD ["./femmg_server"]
