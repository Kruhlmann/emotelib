ARG DEBIAN_VERSION=bookworm

FROM cr.kruhlmann.dev/debian:${DEBIAN_VERSION} AS builder
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    build-essential \
    curl \
    libcurl4-openssl-dev \
    libgd-dev \
    imagemagick \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /home/$USERNAME
USER $USERNAME
COPY . .
RUN make && mv main /usr/local/bin/emotelib

CMD ["emotelib"]
