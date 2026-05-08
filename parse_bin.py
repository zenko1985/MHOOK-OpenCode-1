# -*- coding: utf-8 -*-
import struct

with open('MHookResources.bin', 'rb') as f:
    data = f.read()

with open('names.txt', 'w', encoding='utf-8') as out:
    num = struct.unpack('<I', data[0:4])[0]
    out.write(f'Записей: {num}\n')
    
    pos = 4
    for i in range(num):
        start = pos
        while pos < len(data) and (data[pos] != 0 or data[pos+1] != 0):
            pos += 2
        name = data[start:pos].decode('utf-16-le', errors='replace')
        pos += 2
        
        filetime = struct.unpack('<Q', data[pos:pos+8])[0]
        pos += 8
        
        out.write(f'{i+1}. {name}\n')