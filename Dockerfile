# PHI-OMEGA-ZERO: FEmmg-FHE DOCKER IMAGE
FROM ubuntu:22.04

LABEL maintainer="Dan Joseph M. Fernandez"
LABEL description="FEmmg-FHE v6.5 — Fully Homomorphic Encryption Framework"

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    g++ \
    libssl-dev \
    git \
    wget \
    python3 \
    golang-go \
    && rm -rf /var/lib/apt/lists/*

# Build OpenFHE
RUN git clone https://github.com/openfheorg/openfhe-development.git /opt/openfhe \
    && cd /opt/openfhe \
    && mkdir build && cd build \
    && cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local \
    && make -j$(nproc) \
    && make install \
    && ldconfig

# Copy FEmmg-FHE
COPY . /opt/femmgFHE
WORKDIR /opt/femmgFHE

# Build
RUN make all

# Default command
CMD ["./tests/full_blown_test.sh"]
