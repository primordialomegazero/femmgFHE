FROM ubuntu:22.04 AS builder
RUN apt-get update && apt-get install -y g++ libc6-dev libssl-dev && rm -rf /var/lib/apt/lists/*
WORKDIR /build
COPY src/femmg_fhe.h .
COPY src/fractal_fhe.h .
COPY src/godcode.h .
COPY src/lyapunov_core.h .
COPY src/riemann_deep.h .
COPY src/riemann_zeros_200.h .
COPY src/riemann_zeta.h .
COPY src/phi_stack.h .
COPY src/antimatter.h .
COPY src/metaprogram.h .
COPY src/femmg_server.cpp .
RUN g++ -std=c++17 -O3 -march=native -pthread -static -Wall -Wextra -Werror -o femmg_server femmg_server.cpp -lm -lssl -lcrypto

FROM ubuntu:22.04
WORKDIR /app
COPY --from=builder /build/femmg_server .
EXPOSE 8092
HEALTHCHECK --interval=10s --timeout=3s CMD /app/femmg_server --health-check || exit 1
ENTRYPOINT ["./femmg_server"]
