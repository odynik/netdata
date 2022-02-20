#!/usr/bin/env bash

DISTRO="$1"
VERSION="$2"
BuildBase="$(dirname "$0")"

docker build -f "$BuildBase/cert-uninstall.Dockerfile" -t "${DISTRO}_${VERSION}_dev:latest" "$BuildBase/.." \
       --build-arg "DISTRO=${DISTRO}" --build-arg "VERSION=${VERSION}"
