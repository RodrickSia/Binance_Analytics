# ---- Build stage ----
FROM ubuntu:24.04 AS builder

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    default-jre-headless \
    libboost-dev \
    libssl-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . .

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --target websocket -j"$(nproc)"

# ---- Runtime stage ----
FROM ubuntu:24.04 AS runtime

RUN apt-get update && apt-get install -y --no-install-recommends \
    libssl3 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/* \
    && useradd --system --no-create-home --shell /usr/sbin/nologin appuser

WORKDIR /app
COPY --from=builder /src/build/websocket ./websocket

USER appuser

# BINANCE_API_KEY must be supplied at runtime, e.g. `docker run -e BINANCE_API_KEY=... image`
ENTRYPOINT ["./websocket"]
