FROM ubuntu:22.04 AS builder
RUN apt-get update && apt-get install -y g++ libssl-dev && rm -rf /var/lib/apt/lists/*
WORKDIR /build
COPY src/ src/
COPY security_complete.h .
COPY phi_algo_merge.h .
RUN g++ -std=c++17 -O3 -march=native -pthread -static -Wall -Wextra -Werror \
    -o femmg_server src/femmg_server.cpp -lm -lssl -lcrypto
FROM ubuntu:22.04
RUN apt-get update && apt-get install -y libssl3 && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY --from=builder /build/femmg_server .
EXPOSE 8092
ENV FEMMG_DEV_MODE=0
HEALTHCHECK --interval=10s --timeout=3s CMD /app/femmg_server --health || exit 1
ENTRYPOINT ["./femmg_server"]
