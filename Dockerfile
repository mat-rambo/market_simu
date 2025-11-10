FROM ubuntu:22.04

# Avoid interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && \
    apt-get install -y \
    build-essential \
    cmake \
    git \
    postgresql-client \
    libpq-dev \
    python3 \
    python3-pip \
    netcat \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source files
COPY CMakeLists.txt ./
COPY include/ ./include/
COPY src/ ./src/
COPY web/ ./web/
COPY scripts/ ./scripts/

# Build the project
RUN mkdir build && cd build && \
    cmake .. && \
    make market_simulation && \
    cp market_simulation /app/

# Expose ports
EXPOSE 8888 8080

# Default command
CMD ["./market_simulation", "8888", "8080"]

