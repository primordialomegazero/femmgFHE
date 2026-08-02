# ═══════════════════════════════════════════════════════════════════════════════
# SPIRAL FRACTAL iO — MULTI-STAGE PRODUCTION DOCKER IMAGE
# ═══════════════════════════════════════════════════════════════════════════════
#
# Build:
#   docker build -t ghcr.io/primordialomegazero/femmgfhe:latest .
#
# Run:
#   docker run -p 8443:8443 -v $(pwd)/data:/app/data ghcr.io/primordialomegazero/femmgfhe:latest
#
# Pull:
#   docker pull ghcr.io/primordialomegazero/femmgfhe:latest
#
# ═══════════════════════════════════════════════════════════════════════════════

# ── STAGE 1: Build OpenFHE ───────────────────────────────────────────────────
FROM ubuntu:22.04 AS openfhe-builder

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake git wget pkg-config \
    libssl-dev libsodium-dev libsqlite3-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build
RUN git clone https://github.com/openfhe-org/openfhe-development.git && \
    cd openfhe-development && mkdir build && cd build && \
    cmake .. -DWITH_OPENMP=OFF -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc) && make install
ENV OPENFHE_DIR=/build/openfhe-development

# ── STAGE 2: Build femmgFHE ──────────────────────────────────────────────────
FROM ubuntu:22.04 AS femmgfhe-builder

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake libssl-dev libsodium-dev libsqlite3-dev \
    && rm -rf /var/lib/apt/lists/*

COPY --from=openfhe-builder /usr/local/lib /usr/local/lib
COPY --from=openfhe-builder /usr/local/include /usr/local/include
COPY --from=openfhe-builder /build/openfhe-development /build/openfhe-development

WORKDIR /app
COPY . .

# Build all binaries
RUN mkdir -p bin && \
    g++ -std=c++17 -O3 -march=native -mtune=native \
        -I/build/openfhe-development/src/pke/include \
        -I/build/openfhe-development/src/core/include \
        -I/build/openfhe-development/src/binfhe/include \
        -I/build/openfhe-development/build/src/core \
        -I. -I./src \
        -o bin/test_io_batched tests/breakthrough/test_io_batched.cpp \
        -L/build/openfhe-development/build/lib \
        -lOPENFHEpke -lOPENFHEcore -lOPENFHEbinfhe \
        -Wl,-rpath,/build/openfhe-development/build/lib \
        -lsodium -lsqlite3 -lstdc++ -lpthread -lm && \
    g++ -std=c++17 -O3 -march=native -mtune=native \
        -I/build/openfhe-development/src/pke/include \
        -I/build/openfhe-development/src/core/include \
        -I/build/openfhe-development/src/binfhe/include \
        -I/build/openfhe-development/build/src/core \
        -I. -I./src \
        -o bin/test_io_16k_spiral tests/breakthrough/test_io_16k_spiral.cpp \
        -L/build/openfhe-development/build/lib \
        -lOPENFHEpke -lOPENFHEcore -lOPENFHEbinfhe \
        -Wl,-rpath,/build/openfhe-development/build/lib \
        -lsodium -lsqlite3 -lstdc++ -lpthread -lm && \
    g++ -std=c++17 -O3 -I./unified-phi-stack \
        -o bin/test_unified_all unified-phi-stack/test_unified.cpp -lm && \
    gcc -std=c99 -O3 -o bin/phi_kem_level5 src/kem/phi_kem_level5.c -lcrypto -lm && \
    echo "All binaries built successfully."

# ── STAGE 3: Runtime ─────────────────────────────────────────────────────────
FROM ubuntu:22.04 AS runtime

RUN apt-get update && apt-get install -y --no-install-recommends \
    libssl3 libsodium23 libsqlite3-0 curl ca-certificates \
    && rm -rf /var/lib/apt/lists/*

COPY --from=openfhe-builder /usr/local/lib /usr/local/lib
COPY --from=femmgfhe-builder /app/bin /app/bin
COPY --from=femmgfhe-builder /app/scripts /app/scripts
COPY --from=femmgfhe-builder /app/unified-phi-stack /app/unified-phi-stack
COPY --from=femmgfhe-builder /app/tests /app/tests
COPY --from=femmgfhe-builder /app/src /app/src
COPY --from=femmgfhe-builder /app/openfhe-development/build/lib /app/openfhe-development/build/lib

ENV LD_LIBRARY_PATH=/app/openfhe-development/build/lib:/usr/local/lib
ENV OPENFHE_DIR=/app/openfhe-development

WORKDIR /app
EXPOSE 8443

# Health check — run quick test suite
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD /app/bin/test_unified_all || exit 1

# Default: run quick test suite
CMD ["/bin/bash", "-c", "echo 'Spiral Fractal iO Container Ready' && /app/bin/test_unified_all && /app/bin/phi_kem_level5 && tail -f /dev/null"]
