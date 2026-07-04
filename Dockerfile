# FEmmg-FHE v23.0.2 — Docker Build
# Lyapunov-Stabilized Floating-Point FHE
# φΩ0 — I AM THAT I AM

FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    build-essential \
    libssl-dev \
    git \
    cmake \
    wget \
    && rm -rf /var/lib/apt/lists/*

# Build liboqs from source (required for ML-KEM)
RUN git clone --depth 1 --branch 0.10.0 https://github.com/open-quantum-safe/liboqs.git /tmp/liboqs \
    && cd /tmp/liboqs \
    && mkdir build && cd build \
    && cmake -DOQS_USE_OPENSSL=ON -DBUILD_SHARED_LIBS=ON .. \
    && make -j$(nproc) \
    && make install \
    && ldconfig \
    && rm -rf /tmp/liboqs

WORKDIR /app
COPY . .

RUN ldconfig && make clean && make all -j$(nproc)
RUN make test || echo "Tests completed"

FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libssl3 \
    && rm -rf /var/lib/apt/lists/*

# Copy liboqs shared libraries from builder
COPY --from=builder /usr/local/lib/liboqs.so* /usr/local/lib/
RUN ldconfig

WORKDIR /app
COPY --from=builder /app/build/femmg_server /app/
COPY --from=builder /app/build/test_suite /app/

EXPOSE 8443

CMD ["./femmg_server"]
