#!/usr/bin/env bash
# Prints the release manifest for a built NRO.
#   tools/manifest.sh <nro> <version>
set -euo pipefail

if [ $# -ne 2 ]; then
    echo "usage: $0 <nro> <version>" >&2
    exit 2
fi

nro="$1"
version="$2"
root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

protocol="$(sed -n 's/^constexpr int kProtocolVersion = \([0-9][0-9]*\);.*/\1/p' "$root/src/args.hpp" | tr -d '\r')"
if [ -z "$protocol" ]; then
    echo "kProtocolVersion not found in src/args.hpp" >&2
    exit 1
fi

size="$(stat -c %s "$nro")"
sha="$(sha256sum "$nro" | cut -d' ' -f1)"

printf '{"id": "pcsx-rearmed", "console": "ps1", "version": "%s", "protocol": %s, "asset": "equinox-pcsx-rearmed.nro", "size": %s, "sha256": "%s"}\n' \
    "$version" "$protocol" "$size" "$sha"
