# syntax=docker/dockerfile:1.7

ARG DEBIAN_VERSION=bookworm
FROM debian:${DEBIAN_VERSION}-slim AS build

ARG DFB_PE_TARGETS=x86,x64

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        ca-certificates \
        cmake \
        file \
        mingw-w64 \
        ninja-build \
        python3 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . .

RUN python3 tools/validate_manifest.py \
    && python3 tools/generate_registry_from_manifest.py \
    && python3 tools/generate_expected_from_manifest.py

RUN set -eux; \
    mkdir -p /out; \
    old_ifs="$IFS"; \
    IFS=','; \
    for target in ${DFB_PE_TARGETS}; do \
        IFS="$old_ifs"; \
        target="$(printf '%s' "$target" | tr -d '[:space:]')"; \
        case "$target" in \
            x86) preset="win-cross-x86-debug"; out_dir="windows_pe_x86"; build_dir="build/win-cross-x86-debug" ;; \
            x64|amd64) preset="win-cross-debug"; out_dir="windows_pe_x64"; build_dir="build/win-cross-debug" ;; \
            *) echo "Unknown PE target: $target" >&2; exit 2 ;; \
        esac; \
        cmake --preset "$preset"; \
        cmake --build --preset "$preset"; \
        mkdir -p "/out/$out_dir"; \
        cp -a "$build_dir" "/out/$out_dir/build"; \
        cp -a expected "/out/$out_dir/expected"; \
        printf 'PE_TARGET=%s\nCMAKE_PRESET=%s\n' "$target" "$preset" > "/out/$out_dir/build-info.txt"; \
        find "/out/$out_dir/build" -maxdepth 1 -type f -exec file {} \; > "/out/$out_dir/artifact-file-types.txt"; \
        IFS=','; \
    done

FROM scratch AS export
COPY --from=build /out/ /
