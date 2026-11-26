#!/bin/sh
# Builds the Linux libraries inside the Steam Runtime 1 (scout) SDK container, with its gcc 9, so that hl.so and
# client.so need only an old glibc and load on any distro Half-Life runs on. Released builds come from here.
# A plain "make" on the host works for testing, but its glibc requirement is the host's.
#
# Usage, from anywhere (needs docker):
#   linux/build-steamrt.sh          hl.so and client.so, copied into installer/payload
#   linux/build-steamrt.sh rp3d     rebuild the tracked libreactphysics3d.a (only when its sources change; commit it)
#   linux/build-steamrt.sh clean    remove the object files
set -e

# Pinned for reproducible builds: steamrt/scout/sdk/i386, build 1.0.20261021.266054
IMAGE=registry.gitlab.steamos.cloud/steamrt/scout/sdk/i386@sha256:a5a20f9b4b205998a30318259de04a70d98bfbd258750c5f6eb78589d1d37a7d

ROOT=$(cd "$(dirname "$0")/.." && pwd)
DOCKER_TTY=
[ -t 1 ] && DOCKER_TTY=-t

exec docker run --rm $DOCKER_TTY --user "$(id -u):$(id -g)" -v "$ROOT:/src" -w /src/linux "$IMAGE" \
	make CFG=release COMPILER=g++-9 "$@"
