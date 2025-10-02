FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Install Kubuntu desktop
RUN apt-get update && apt-get install -y \
    wget \
    kubuntu-desktop \
    && rm -rf /var/lib/apt/lists/*
