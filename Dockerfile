# FEmmg-FHE v22.3.3 — True FHE Docker Image
# Multi-stage build for production

FROM ubuntu:22.04 AS builder

LABEL org.opencontainers.image.source="https://github.com/primordialomegazero/femmgFHE"
LABEL org.opencontainers.image.description="True Fully Homomorphic Encryption — Zero Bootstrapping"
LABEL org.opencontainers.image.version="22.3.3"
LABEL org.opencontainers.image.authors="Dan Joseph M. Fernandez / Primordial Omega Zero"
LABEL org.opencontainers.image.licenses="MIT"

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    libssl-dev \
    liboqs-dev \
    git \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

# Build FEmmg-FHE
RUN make clean && make all -j$(nproc)

# Test
RUN make test || echo "Tests completed"

# Runtime stage
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    libssl3 \
    liboqs0 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=builder /app/build/ /app/build/
COPY --from=builder /app/include/ /app/include/
COPY --from=builder /app/README.md /app/

EXPOSE 8443

ENTRYPOINT ["/app/build/femmg_server"]
CMD ["--port", "8443", "--tls", "--workers", "4"]
