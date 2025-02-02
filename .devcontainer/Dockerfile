# See here for image contents: https://github.com/microsoft/vscode-dev-containers/tree/v0.148.1/containers/cpp/.devcontainer/base.Dockerfile

# [Choice] Debian / Ubuntu version: debian-10, debian-9, ubuntu-20.04, ubuntu-18.04
ARG VARIANT="ubuntu-20.04"
FROM mcr.microsoft.com/vscode/devcontainers/cpp:0-${VARIANT}

RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install --no-install-recommends \
      build-essential \
      cmake \
      gettext \
      ninja-build \
      libcurl4-openssl-dev \
      libgspell-1-dev \
      libgtkmm-3.0-dev \
      libgtksourceview-4-dev \
      libsqlite3-dev \
      libuchardet-dev \
      libfmt-dev \
      libspdlog-dev \
      libxml++2.6-dev \
      libxml2-utils \
      libuchardet-dev \
      libfribidi-dev \
      libvte-2.91-dev
