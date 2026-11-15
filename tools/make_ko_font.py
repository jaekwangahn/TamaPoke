#!/usr/bin/env python3
"""Build compact 12/16-pixel Korean U8g2 fonts from pinned Noto Sans KR.

Requires Pillow and fonttools. Source font is cached, not committed.
SIL Open Font License: docs/fonts/NotoSansKR-OFL.txt.
The firmware's existing U8g2 encoder/decoder is reused and round-trip checked.
"""
from pathlib import Path
import hashlib
import re
import urllib.request

from fontTools.ttLib import TTFont
from PIL import Image, ImageDraw, ImageFont
import make_zh_font as codec
import test_i18n_formats as fmt

ROOT = Path(__file__).resolve().parents[1]
FONT_REV = 'b38c5c93af322c45f633e17ac440ec1e6c94d489'
FONT_URL = f'https://raw.githubusercontent.com/google/fonts/{FONT_REV}/ofl/notosanskr/NotoSansKR[wght].ttf'
FONT_PATH = ROOT / 'tools/downloads/fonts/NotoSansKR.ttf'
FONT_SHA256 = '194018e6b2b293a7964f037b25c0249ce1418bc9ab3c971060a03aa57861e252'


def used_chars():
    source = (ROOT/'i18n.cpp').read_text()
    ko = fmt.LANGS.index('KO')
    texts = []
    for table in ('STRINGS','MED_NAME','MED_LBL','MED_DSC'):
        texts += [literal[1:-1] for literal in fmt.extract_table(source,table,len(fmt.LANGS))[ko]]
    texts += re.findall(r'"([^"\n]*)"',(ROOT/'dex_names_ko.h').read_text())
    for path in ROOT.glob('*.ino'):
        source = re.sub(r'//[^\n]*','',path.read_text())
        texts += re.findall(r'"([^"\n]*)"',source)
    return sorted({ord(char) for text in texts for char in text if ord(char)>126})


def glyph(face,char):
    left,top,right,bottom = face.getbbox(char,anchor='ls')
    width,height = right-left,bottom-top
    advance = round(face.getlength(char))
    if not width or not height:
        return (0,0,0,0,advance,[])
    image = Image.new('L',(width,height))
    ImageDraw.Draw(image).text((-left,-top),char,font=face,fill=255,anchor='ls')
    rows = []
    for y in range(height):
        row = 0
        for x in range(width):
            row = row*2 + (image.getpixel((x,y))>=112)
        rows.append(row)
    return codec.trim((width,height,left,-bottom,advance,rows))


def main():
    if not FONT_PATH.exists():
        FONT_PATH.parent.mkdir(parents=True,exist_ok=True)
        with urllib.request.urlopen(FONT_URL,timeout=90) as response:
            FONT_PATH.write_bytes(response.read())
    assert hashlib.sha256(FONT_PATH.read_bytes()).hexdigest()==FONT_SHA256, 'Unexpected source font revision'
    cmap = TTFont(FONT_PATH).getBestCmap()
    codes = list(range(32,127))+used_chars()
    missing = [chr(code) for code in codes if code not in cmap]
    if missing:
        raise SystemExit('Missing Korean font characters: '+''.join(missing))
    output = ['#pragma once','#include <stdint.h>','',
              '// GENERADO by tools/make_ko_font.py; do not edit.',
              f'// Noto Sans KR, Google Fonts revision {FONT_REV}.',
              f'// Source SHA256: {hashlib.sha256(FONT_PATH.read_bytes()).hexdigest()}',
              '// SIL OFL 1.1: docs/fonts/NotoSansKR-OFL.txt.',
              '// Bitmap subsets are named TamaPoke Korean (modified font).','']
    for size in (12,16):
        face = ImageFont.truetype(str(FONT_PATH),size)
        face.set_variation_by_axes([500])
        selected = {code:glyph(face,chr(code)) for code in codes}
        encoded = codec.build_font(selected)
        for code in codes:
            assert codec.decode_font(encoded,code)==selected[code],chr(code)
        output.append(f'static const uint8_t u8g2_font_tp_ko{size}[{len(encoded)}] = {{')
        output += ['  '+', '.join(f'0x{value:02x}' for value in encoded[pos:pos+24])+','
                   for pos in range(0,len(encoded),24)]
        output += ['};','']
        print(f'Korean {size}px: {len(encoded)} bytes, {len(codes)} glyphs',flush=True)
    (ROOT/'kofont.h').write_text('\n'.join(output))
    (ROOT/'tools/ko_font_chars.txt').write_text(''.join(chr(code) for code in used_chars())+'\n')


if __name__=='__main__':
    main()
