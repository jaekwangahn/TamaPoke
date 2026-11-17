#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."
target=smalltv-pro
cpp_flags=-DTAMAPOKE_SMALLTV_PRO
case "${1:-}" in
  --ota)
    target=smalltv-pro-ota
    cpp_flags="$cpp_flags -DTAMAPOKE_SMALLTV_OTA"
    ;;
  "") ;;
  *) echo "usage: $0 [--ota]" >&2; exit 2 ;;
esac

mkdir -p "build/$target"
python3 tools/pack_smalltv_thumbs.py tools/sdcard/mons/thumbs.bin "build/$target/thumbs.bin"
fqbn='esp32:esp32:esp32:FlashSize=8M,PartitionScheme=default_8MB,PSRAM=disabled'
arduino-cli compile --fqbn "$fqbn" \
  --build-property "compiler.cpp.extra_flags=$cpp_flags" \
  --build-property "compiler.S.extra_flags=-DTAMAPOKE_SMALLTV_PRO -Xassembler -I -Xassembler \"$PWD/build/$target\"" \
  --build-path "build/$target-work" \
  --output-dir "build/$target" .

if [[ "$target" == smalltv-pro-ota ]]; then
  bytes=$(wc -c < "build/$target/TamaPoke.ino.bin")
  if (( bytes > 2228224 )); then
    echo "Application exceeds the 2,228,224-byte SmallTV PRO OTA slot" >&2
    exit 1
  fi
fi
