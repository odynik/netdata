#!/usr/bin/env bash

DISTRO="$1"
VERSION="$2"
BuildBase="$(cd "$(dirname "$0")" && cd .. && pwd)"

docker build -f "$BuildBase/make-install.Dockerfile" -t "${DISTRO}_${VERSION}_master:latest" "$BuildBase/.." \
       --build-arg "DISTRO=${DISTRO}" --build-arg "VERSION=${VERSION}"
