# Start with Alpine base
FROM alpine:3.21.3

# Install required packages for C++ development
RUN apk update && apk add --no-cache \
    g++ \
    make \
    cmake \
    bash \
    git \
    vim \
    linux-headers

# Set working directory inside the container
WORKDIR /workspace

# To run the dev container in docker cmd+shft+p
