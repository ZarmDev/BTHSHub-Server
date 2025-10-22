FROM archlinux:base as build

# Update and install build dependencies
RUN pacman -Syu --noconfirm && pacman -S --noconfirm \
    base-devel \
    cmake \
    ninja \
    git \
    pkg-config \
    libsodium \
    curl \
    zip \
    unzip \
    tar \
    openssl \
    hiredis \
    python \
    && echo "Dependencies installed successfully"

# Set working directory
WORKDIR /app

# Install vcpkg
RUN git clone https://github.com/microsoft/vcpkg.git && \
    echo "vcpkg repository cloned" && \
    ./vcpkg/bootstrap-vcpkg.sh && \
    echo "vcpkg bootstrapped successfully"

# Install packages with vcpkg
RUN ./vcpkg/vcpkg install --triplet=x64-linux hiredis jwt-cpp redis-plus-plus nlohmann-json podofo && \
    echo "vcpkg packages installed successfully"

# Install redis-plus-plus manually
RUN git clone https://github.com/sewenew/redis-plus-plus.git && \
    cd redis-plus-plus && \
    mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    make -j$(nproc) && \
    make install && \
    echo "redis-plus-plus installed manually"

# Copy project files
COPY . . 
RUN echo "Project files copied"

# Important: Make sure the build directory is completely clean
RUN rm -rf build && \
    mkdir -p build && \
    echo "Clean build directory created"

# Build the application with clean directory
RUN cd build && \
    cmake .. -DCMAKE_TOOLCHAIN_FILE=/app/vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Release && \
    echo "CMake configuration successful" && \
    cmake --build . --verbose && \
    echo "Build completed successfully"

# Runtime stage
# FROM archlinux:base
RUN pacman -Rns --noconfirm base-devel cmake ninja git \
    && pacman -Scc --noconfirm

# Install runtime dependencies
# RUN pacman -Syu --noconfirm && pacman -S --noconfirm \
#     libsodium \
#     hiredis \
#     ca-certificates && \
#     pacman -Scc --noconfirm

WORKDIR /app

# Set PORT environment variable for Heroku
ENV PORT=8080

# Command to run - note the path change since we're already in the right directory
CMD ./build/server

# Expose the port
EXPOSE 8080