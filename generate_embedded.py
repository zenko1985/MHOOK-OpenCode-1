import os
import struct

SOURCE_DIR = r"C:\Programs\mhook"
OUTPUT_FILE = "EmbeddedSettings.bin"

files = []
for filename in os.listdir(SOURCE_DIR):
    if filename.lower().endswith('.mhook'):
        filepath = os.path.join(SOURCE_DIR, filename)
        if os.path.isfile(filepath):
            stat = os.stat(filepath)
            files.append({
                'name': filename,
                'mtime': stat.st_mtime
            })

# Сортировка по дате (новые первые)
files.sort(key=lambda x: -x['mtime'])

print(f"Found {len(files)} .mhook files")

with open(OUTPUT_FILE, 'wb') as f:
    f.write(struct.pack('I', len(files)))
    
    for file_info in files:
        name_bytes = file_info['name'].encode('utf-16-le')
        name_len = len(name_bytes) // 2
        
        f.write(struct.pack('I', name_len))
        f.write(name_bytes)
        f.write(struct.pack('Q', int(file_info['mtime'] * 10000000)))

print(f"Created {OUTPUT_FILE} with {len(files)} files")
