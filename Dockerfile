FROM ubuntu:22.04 AS builder
RUN apt-get update && apt-get install -y g++ libc6-dev && rm -rf /var/lib/apt/lists/*
WORKDIR /build
COPY src/femmg_fhe.h .
COPY src/fractal_fhe.h .
COPY src/godcode.h .
COPY src/lyapunov_core.h .
COPY src/lyapunov_stabilizer.h .
COPY src/femmg_server.cpp .
RUN g++ -std=c++17 -O3 -march=native -pthread -static -o femmg_server femmg_server.cpp -lm

FROM ubuntu:22.04
WORKDIR /app
COPY --from=builder /build/femmg_server .
EXPOSE 8092
HEALTHCHECK --interval=10s --timeout=3s CMD /app/femmg_server --health-check || exit 1
ENTRYPOINT ["./femmg_server"]
