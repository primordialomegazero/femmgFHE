# ═══════════════════════════════════════════════════════════════════════════════
# SPIRAL FRACTAL iO — PRODUCTION DOCKER IMAGE
# ═══════════════════════════════════════════════════════════════════════════════
#
# Build:
#   docker build -t spiralfractalio .
#
# Run:
#   docker run -p 8443:8443 -v $(pwd)/data:/data spiralfractalio
#
# Troubleshooting: see TROUBLESHOOTING.md
#
# ═══════════════════════════════════════════════════════════════════════════════

# ── STAGE 1: Build OpenFHE ───────────────────────────────────────────────────
FROM ubuntu:22.04 AS openfhe-builder

RUN apt-get update && apt-get install -y \
    build-essential cmake git wget pkg-config \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build
RUN git clone https://github.com/openfhe-org/openfhe-development.git && \
    cd openfhe-development && mkdir build && cd build && \
    cmake .. -DWITH_OPENMP=OFF -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc) && make install

# ── STAGE 2: Install Dependencies ─────────────────────────────────────────────
FROM ubuntu:22.04 AS dependencies

RUN apt-get update && apt-get install -y \
    build-essential cmake git \
    libsodium-dev libsqlite3-dev libssl-dev \
    wget pkg-config \
    && rm -rf /var/lib/apt/lists/*

# Install liboqs (Open Quantum Safe — for HydraJWT PQ heads)
RUN git clone --depth 1 https://github.com/open-quantum-safe/liboqs.git && \
    cd liboqs && mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc) && make install && \
    ldconfig

# Copy OpenFHE from stage 1
COPY --from=openfhe-builder /usr/local/include/openfhe /usr/local/include/openfhe
COPY --from=openfhe-builder /usr/local/lib/libOPENFHE* /usr/local/lib/

# ── STAGE 3: Build femmgFHE ───────────────────────────────────────────────────
FROM dependencies AS builder

WORKDIR /app
COPY . .

# Build HydraJWT (PQ auth library)
RUN cd archive/HydraJWT && \
    mkdir -p build && cd build && \
    cmake .. && make -j$(nproc)

# Build femmgFHE core
RUN g++ -std=c++17 -O3 -march=native -mtune=native \
    -I./openfhe-development/src/pke/include \
    -I./openfhe-development/src/core/include \
    -I./openfhe-development/src/binfhe/include \
    -I./openfhe-development/build/src/core \
    -I. -Iinclude \
    -o bin/test_io_batched \
    tests/breakthrough/test_io_batched.cpp \
    -L./openfhe-development/build/lib \
    -lOPENFHEpke -lOPENFHEcore -lOPENFHEbinfhe \
    -Wl,-rpath,/usr/local/lib \
    -lstdc++ -lpthread -lm

RUN g++ -std=c++17 -O0 -I. -Iinclude \
    -o bin/test_full_integration \
    tests/breakthrough/test_full_integration.cpp \
    bin/libhydrajwt.a \
    -loqs -lssl -lcrypto -lsodium -lsqlite3 -lpthread -lm

# ── STAGE 4: Runtime ──────────────────────────────────────────────────────────
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libsodium23 libsqlite3-0 libssl3 \
    && rm -rf /var/lib/apt/lists/*

# Copy built binaries and libraries
COPY --from=builder /app/bin /app/bin
COPY --from=builder /usr/local/lib /usr/local/lib
COPY --from=builder /usr/local/include /usr/local/include

RUN ldconfig

WORKDIR /app
ENV LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

# Health check
HEALTHCHECK --interval=30s --timeout=10s --retries=3 \
    CMD /app/bin/test_full_integration || exit 1

EXPOSE 8443

# Default: run batched iO test in DEV mode
CMD ["/app/bin/test_io_batched", "10", "3"]
