"""Pack each existing TPTH thumbnail as an independently readable zlib stream."""

import struct
import sys
import zlib
from pathlib import Path


def pack(source: bytes) -> bytes:
    if source[:4] != b"TPTH":
        raise ValueError("expected a TPTH thumbnail bank")
    (count,) = struct.unpack_from("<H", source, 4)
    table_start = 6
    table_end = table_start + 4 * count
    if table_end > len(source):
        raise ValueError("truncated thumbnail index")
    index = bytearray(table_end)
    struct.pack_into("<4sH", index, 0, b"TPZ1", count)
    payload = bytearray()
    for slot, (offset,) in enumerate(struct.iter_unpack("<I", source[table_start:table_end])):
        if not offset:
            continue
        if offset < table_end or offset + 3 > len(source):
            raise ValueError(f"invalid thumbnail offset at slot {slot + 1}")
        width, height, palette = source[offset : offset + 3]
        length = 3 + 2 * palette + width * height
        if length > 2048 or offset + length > len(source):
            raise ValueError(f"invalid thumbnail size at slot {slot + 1}")
        compressed = zlib.compress(source[offset : offset + length], level=9)
        struct.pack_into("<I", index, table_start + 4 * slot, table_end + len(payload))
        payload.extend(struct.pack("<H", len(compressed)))
        payload.extend(compressed)
    return bytes(index + payload)


if __name__ == "__main__":
    if len(sys.argv) != 3:
        sys.exit("usage: pack_smalltv_thumbs.py INPUT OUTPUT")
    Path(sys.argv[2]).write_bytes(pack(Path(sys.argv[1]).read_bytes()))
