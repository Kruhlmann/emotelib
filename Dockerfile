ARG DEBIAN_VERSION=bookworm

FROM cr.kruhlmann.dev/debian:${DEBIAN_VERSION} AS builder
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    build-essential \
    pkg-config \
    curl \
    libcurl4-openssl-dev \
    imagemagick \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/* \
    && ln -s /home/$USERNAME/main /usr/local/bin/emotelib
WORKDIR /home/$USERNAME
COPY . .
RUN make
CMD ["/usr/local/bin/emotelib"]
