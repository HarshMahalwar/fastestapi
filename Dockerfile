FROM ubuntu:24.04 AS base

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        ninja-build \
        pkg-config \
        git \
        ca-certificates \
        libsqlite3-dev \
        libssl-dev \
        libpoco-dev \
    && rm -rf /var/lib/apt/lists/*

FROM base AS builder

WORKDIR /src
COPY . .

RUN cmake -B build -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr/local \
    && cmake --build build --parallel \
    && cmake --install build

FROM base AS runtime

COPY --from=builder /usr/local /usr/local

WORKDIR /app

RUN ldconfig && g++ --version && cmake --version
