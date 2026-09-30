# -*- coding: utf-8 -*-
# Генератор EmbeddedBitmaps.bin — встроенные в exe битмапы джойстика.
#
# Зачем: исходные bm*.bmp занимают 480 КБ каждый (1,92 МБ всего), что составляет
# почти 3/4 размера экзешника. Упаковываем deflate прямо в тот же формат
# контейнера, что уже используется для EmbeddedSettings.bin ('ZLIB' + размер),
# распаковка на стороне C++ выполняется через miniz (см. RecentFiles.cpp).
#
# Формат контейнера (до сжатия):
#   DWORD  count                 количество картинок
#   для каждой картинки:
#     DWORD width
#     DWORD height
#     DWORD offset               смещение пикселей внутри распакованных данных
#     DWORD size                 длина пикселей в байтах
#   пиксели: 24 бита BGR, строки снизу вверх (как в .bmp), шаг строки
#            ((width*3+3)/4)*4, выровнено по 4 байта
#
# Использование:
#   python generate_embedded_bitmaps.py            собрать EmbeddedBitmaps.bin из bm*.bmp
#   python generate_embedded_bitmaps.py --extract  восстановить bm*.bmp из контейнера
#
# Исходные bm*.bmp намеренно не хранятся в дереве исходников: контейнер содержит
# те же 24-битные пиксели без потерь, поэтому он единственный источник правды,
# а .bmp восстанавливаются из него одной командой. Внести правку в картинку:
#   1) python generate_embedded_bitmaps.py --extract
#   2) отредактировать bm*.bmp
#   3) python generate_embedded_bitmaps.py
#   4) удалить bm*.bmp, если они больше не нужны

import os
import struct
import sys
import zlib

SOURCE_DIR = os.path.dirname(os.path.abspath(__file__))
OUTPUT_FILE = os.path.join(SOURCE_DIR, "EmbeddedBitmaps.bin")

ZLIB_MAGIC = b'ZLIB'

# Порядок должен совпадать с порядком в Bitmap.cpp (MH_BITMAP_COUNT)
BITMAPS = ["bm4w.bmp", "bm4wred.bmp", "bm8w.bmp", "bm8wred.bmp"]


def read_bmp(path):
    """Возвращает (width, height, pixels) для 24-битного BI_RGB .bmp без сжатия."""
    with open(path, "rb") as f:
        data = f.read()
    if data[:2] != b'BM':
        raise ValueError("%s: не файл BMP" % path)
    pixel_offset = struct.unpack_from('<I', data, 10)[0]
    header_size = struct.unpack_from('<I', data, 14)[0]
    if header_size != 40:
        raise ValueError("%s: неподдерживаемый заголовок %d" % (path, header_size))
    width = struct.unpack_from('<i', data, 18)[0]
    height = struct.unpack_from('<i', data, 22)[0]
    bpp = struct.unpack_from('<H', data, 28)[0]
    compression = struct.unpack_from('<I', data, 30)[0]
    if bpp != 24 or compression != 0:
        raise ValueError("%s: ожидался 24 бита без сжатия, получено %d/%d"
                         % (path, bpp, compression))
    if height < 0:
        raise ValueError("%s: верхние строки (height<0) не поддерживаются" % path)
    stride = ((width * 3 + 3) // 4) * 4
    expected = pixel_offset + stride * height
    if len(data) < expected:
        raise ValueError("%s: файл короче %d байт" % (path, expected))
    return width, height, data[pixel_offset:expected]


def build_payload(entries):
    """Собирает распакованный контейнер: заголовок + пиксели."""
    header = struct.pack('<I', len(entries))
    table_size = 4 + 16 * len(entries)
    offset = (table_size + 3) & ~3  # выравнивание по 4 байта
    for entry in entries:
        entry['offset'] = offset
        header += struct.pack('<IIII', entry['width'], entry['height'],
                              offset, entry['size'])
        offset += entry['size']
    payload = bytearray(offset)
    payload[0:len(header)] = header
    for entry in entries:
        pos = entry['offset']
        payload[pos:pos + len(entry['pixels'])] = entry['pixels']
    return bytes(payload)


def parse_container(blob):
    """Разбирает EmbeddedBitmaps.bin обратно в список картинок с пикселями."""
    if blob[:4] != ZLIB_MAGIC:
        raise ValueError("EmbeddedBitmaps.bin: нет магии 'ZLIB'")
    orig_size = struct.unpack_from('<I', blob, 4)[0]
    payload = zlib.decompress(blob[8:])
    if len(payload) != orig_size:
        raise ValueError("EmbeddedBitmaps.bin: размер распакованных данных %d != %d"
                         % (len(payload), orig_size))
    count = struct.unpack_from('<I', payload, 0)[0]
    if count != len(BITMAPS):
        raise ValueError("EmbeddedBitmaps.bin: картинок %d, ожидалось %d"
                         % (count, len(BITMAPS)))
    entries = []
    for i, name in enumerate(BITMAPS):
        w, h, offset, size = struct.unpack_from('<IIII', payload, 4 + 16 * i)
        stride = ((w * 3 + 3) // 4) * 4
        if size != stride * h:
            raise ValueError("%s: размер пикселей %d != %d" % (name, size, stride * h))
        if offset + size > len(payload):
            raise ValueError("%s: пиксели выходят за границу данных" % name)
        entries.append({'name': name, 'width': w, 'height': h,
                        'size': size, 'pixels': payload[offset:offset + size]})
    return entries


def write_bmp(path, width, height, pixels):
    """Пишет 24-битный BI_RGB .bmp без палитры (54 байта заголовка)."""
    stride = ((width * 3 + 3) // 4) * 4
    if len(pixels) != stride * height:
        raise ValueError("пикселей %d, ожидалось %d" % (len(pixels), stride * height))
    offset = 14 + 40
    file_header = b'BM' + struct.pack('<IHHI', offset + len(pixels), 0, 0, offset)
    info_header = struct.pack('<IiiHHIIiiII', 40, width, height, 1, 24, 0,
                              len(pixels), 2835, 2835, 0, 0)
    with open(path, "wb") as f:
        f.write(file_header + info_header + pixels)


def extract():
    with open(OUTPUT_FILE, "rb") as f:
        blob = f.read()
    entries = parse_container(blob)
    for entry in entries:
        path = os.path.join(SOURCE_DIR, entry['name'])
        write_bmp(path, entry['width'], entry['height'], entry['pixels'])
        print("восстановлен %-12s %4dx%-4d %7d байт"
              % (entry['name'], entry['width'], entry['height'], entry['size']))
    print("")
    print("Восстановлено %d файлов из %s" % (len(entries), os.path.basename(OUTPUT_FILE)))


def main():
    entries = []
    for name in BITMAPS:
        path = os.path.join(SOURCE_DIR, name)
        width, height, pixels = read_bmp(path)
        entries.append({'name': name, 'width': width, 'height': height,
                        'size': len(pixels), 'pixels': pixels})
        print("%-12s %4dx%-4d %3d бит  %7d байт"
              % (name, width, height, 24, len(pixels)))

    payload = build_payload(entries)
    compressed = zlib.compress(payload, 9)
    blob = ZLIB_MAGIC + struct.pack('<I', len(payload)) + compressed
    with open(OUTPUT_FILE, "wb") as f:
        f.write(blob)

    total = sum(e['size'] for e in entries)
    print("")
    print("Исходные .bmp : %7d байт" % total)
    print("Контейнер     : %7d байт (%.1f%%)"
          % (len(payload), 100.0 * len(payload) / total))
    print("EmbeddedBitmaps.bin : %7d байт (%.1f%%)"
          % (len(blob), 100.0 * len(blob) / total))
    print("Экономия      : %7d байт" % (total - len(blob)))


if __name__ == "__main__":
    if "--extract" in sys.argv:
        extract()
    else:
        main()
