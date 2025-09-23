FROM debian:bullseye-slim as build

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    pkg-config \
    libsodium-dev \
    curl \
    zip \
    unzip \
    tar \
    libssl-dev \
    libz-dev

# Set working directory
WORKDIR /app

# Install vcpkg
RUN git clone https://github.com/microsoft/vcpkg.git && \
    ./vcpkg/bootstrap-vcpkg.sh

# Copy project files
COPY . .

# Install dependencies and build
RUN ./vcpkg/vcpkg install --triplet=x64-linux && \
    mkdir -p build && \
    cd build && \
    cmake .. -DCMAKE_TOOLCHAIN_FILE=/app/vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Release && \
    cmake --build .

# Runtime stage
FROM debian:bullseye-slim

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libsodium23 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy the built executable
COPY --from=build /app/build/server /app/

# Copy required files
COPY --from=build /app/.env /app/

# Expose the port
ENV PORT=4221
EXPOSE 4221

# Command to run
CMD ["/app/server"]