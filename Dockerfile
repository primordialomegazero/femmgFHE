# ╔══════════════════════════════════════════════════════════════╗
# ║  FEmmG-FHE Docker Image v22.4                              ║
# ║  ZANS Production Library + SEAL 4.3 + OpenSSL              ║
# ║  PHI-OMEGA-ZERO — I AM THAT I AM                          ║
# ╚══════════════════════════════════════════════════════════════╝

FROM ubuntu:22.04

LABEL org.opencontainers.image.title="FEmmG-FHE"
LABEL org.opencontainers.image.description="Zero-Anchor Noise Stabilization and Fibonacci-Decomposed Multiplication for Bootstrapping-Free FHE"
LABEL org.opencontainers.image.authors="Dan Joseph M. Fernandez"
LABEL org.opencontainers.image.url="https://github.com/primordialomegazero/femmgFHE"
LABEL org.opencontainers.image.version="22.4.0"
LABEL org.opencontainers.image.licenses="MIT"

# Install all dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    g++ \
    cmake \
    git \
    libgmp-dev \
    libntl-dev \
    libssl-dev \
    libcurl4-openssl-dev \
    ca-certificates \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Install Microsoft SEAL 4.3
WORKDIR /opt
RUN git clone --depth 1 --branch v4.3.3 https://github.com/microsoft/SEAL.git seal && \
    cd seal && \
    cmake -S . -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DSEAL_BUILD_EXAMPLES=OFF \
        -DSEAL_BUILD_TESTS=OFF && \
    cmake --build build -j$(nproc) && \
    cmake --install build && \
    rm -rf build

# Copy FEmmG-FHE source
WORKDIR /opt/femmg-fhe
COPY . .

# Build server with OpenSSL
RUN g++ -std=c++17 -O2 -march=native \
    src/server/femmg_server.cpp \
    -I /usr/local/include/SEAL-4.3 \
    /usr/local/lib/libseal-4.3.a \
    -lssl -lcrypto \
    -pthread -o /opt/femmg-fhe/build/femmg_server

# Create build directory
RUN mkdir -p /opt/femmg-fhe/build

# Expose port
EXPOSE 8092

# Health check
HEALTHCHECK --interval=30s --timeout=10s --retries=3 \
    CMD curl -f http://localhost:8092/health || exit 1

# Run server
CMD ["/opt/femmg-fhe/build/femmg_server"]
