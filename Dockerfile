FROM alpine:3.19 AS builder
RUN apk add --no-cache g++ libc-dev openssl-dev
WORKDIR /build
COPY src/ src/
COPY security_complete.h .
COPY phi_algo_merge.h .
RUN g++ -std=c++17 -O3 -march=native -pthread -static -Wall -Wextra -Werror \
    -o femmg_server src/femmg_server.cpp -lm -lssl -lcrypto
FROM alpine:3.19
RUN apk add --no-cache libstdc++ openssl
WORKDIR /app
COPY --from=builder /build/femmg_server .
EXPOSE 8092
ENV FEMMG_DEV_MODE=0
HEALTHCHECK --interval=10s --timeout=3s CMD /app/femmg_server --health || exit 1
ENTRYPOINT ["./femmg_server"]
