#!/usr/bin/env bash
set -euo pipefail
ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
TMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TMP_DIR"' EXIT

cat > "$TMP_DIR/sample.dot" <<'DOT'
.html
..body @:body1 class:main #:ram,ddr4
...h1 .:Hello_World
DOT

"$ROOT_DIR/build/dot_dump" "$TMP_DIR/sample.dot" | grep -q ".html"
echo "smoke ok"
