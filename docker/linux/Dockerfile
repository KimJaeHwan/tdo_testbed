# syntax=docker/dockerfile:1.7

ARG DEBIAN_VERSION=bookworm
FROM debian:${DEBIAN_VERSION}-slim AS build

ARG TARGETPLATFORM
ARG TARGETARCH
ARG TARGETVARIANT
ARG DFB_RUN_SMOKE=1

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        cmake \
        file \
        ninja-build \
        python3 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . .

RUN python3 tools/validate_manifest.py \
    && python3 tools/generate_registry_from_manifest.py \
    && python3 tools/generate_expected_from_manifest.py

RUN cmake --preset linux-docker-debug \
    && cmake --build --preset linux-docker-debug

RUN if [ "${DFB_RUN_SMOKE}" = "1" ]; then python3 tests/smoke_test.py; fi

RUN mkdir -p /out \
    && cp -a build/docker-debug /out/build \
    && cp -a expected /out/expected \
    && printf 'TARGETPLATFORM=%s\nTARGETARCH=%s\nTARGETVARIANT=%s\n' \
        "${TARGETPLATFORM}" "${TARGETARCH}" "${TARGETVARIANT}" \
        > /out/build-info.txt \
    && find /out/build -maxdepth 1 -type f -exec file {} \; \
        > /out/artifact-file-types.txt

FROM scratch AS export
COPY --from=build /out/ /
