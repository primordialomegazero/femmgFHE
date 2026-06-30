# FEmmg-FHE v21.0.1 — Docker Image
# Multi-stage build for minimal size

# ============================================================
# STAGE 1: Build
# ============================================================
FROM alpine:latest AS builder

RUN apk add --no-cache \
    g++ \
    make \
    libc-dev \
    openssl-dev \
    openssl-libs-static \
    musl-dev \
    musl-static \
    linux-headers \
    zlib-static

WORKDIR /build

# Copy source
COPY src/ ./src/
COPY security_complete.h .
COPY phi_algo_merge.h .

# Build server - remove -static for now, use musl-static
RUN g++ -std=c++17 -O3 -march=native \
    src/femmg_server.cpp \
    -o femmg_server \
    -lm -lssl -lcrypto -lpthread \
    -static-libstdc++ -static-libgcc

# ============================================================
# STAGE 2: Runtime
# ============================================================
FROM alpine:latest

RUN apk add --no-cache \
    libstdc++ \
    libcrypto3 \
    libssl3

WORKDIR /app

# Copy binary from builder
COPY --from=builder /build/femmg_server .

# Expose port
EXPOSE 8092

# Health check
HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
    CMD ./femmg_server --health || exit 1

# Run server
ENTRYPOINT ["./femmg_server"]
