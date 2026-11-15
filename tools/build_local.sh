#!/usr/bin/env bash
# Build from any checkout directory name (Arduino requires TamaPoke/TamaPoke.ino).
set -euo pipefail
shopt -s nullglob
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SKETCH="$ROOT/build/sketch/TamaPoke"
mkdir -p "$SKETCH" "$ROOT/out"
# Remove old staged source files when switching between upstream and Expanded.
find "$SKETCH" -maxdepth 1 -type f \( -name '*.ino' -o -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) -delete
cp "$ROOT"/*.ino "$ROOT"/*.cpp "$ROOT"/*.h "$ROOT"/*.hpp "$SKETCH/"
if [[ -d "$ROOT/boards" ]]; then
  cp -R "$ROOT/boards" "$SKETCH/"
fi
arduino-cli compile \
  --fqbn 'esp32:esp32:esp32s3:CDCOnBoot=cdc,FlashSize=16M,PSRAM=opi,PartitionScheme=app3M_fat9M_16MB' \
  --output-dir "$ROOT/out" "$@" "$SKETCH"
