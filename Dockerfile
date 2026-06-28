FROM gcc:13-bookworm AS builder
WORKDIR /app
COPY src/ .
RUN g++ -std=c++17 -O3 -march=native -pthread -static -o femmg_server femmg_server.cpp -lm

FROM debian:bookworm-slim
WORKDIR /app
COPY --from=builder /app/femmg_server .
EXPOSE 8092
CMD ["./femmg_server"]
