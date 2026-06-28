# Stage 1: Build
FROM gcc:13-bookworm AS builder

WORKDIR /app
COPY src/hydra_final.cpp .

# Build with full optimization
RUN g++ -std=c++17 -O3 -march=native -pthread -static -o FEmmg_fhe hydra_final.cpp

# Stage 2: Minimal runtime
FROM debian:bookworm-slim

WORKDIR /app
COPY --from=builder /app/FEmmg_fhe .

EXPOSE 8092

# Run with 12 threads (override with -e THREADS=N)
ENV THREADS=12

CMD ["./FEmmg_fhe"]
