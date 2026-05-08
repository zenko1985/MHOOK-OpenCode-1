import struct
import re

with open('EmbeddedSettings.bin', 'rb') as f:
    data = f.read()

num = struct.unpack('<I', data[0:4])[0]
all_keys = set()

pos = 4
errors = 0

for i in range(min(1000, num)):
    try:
        name_len = struct.unpack('<I', data[pos:pos+4])[0]
        pos += 4 + name_len*2 + 8 + 4
        content_len = struct.unpack('<I', data[pos:pos+4])[0]
        pos += 4
        content = data[pos:pos+content_len]
        pos += content_len
        
        text = content.decode('utf-8', errors='replace')
        for line in text.split('\r\n'):
            m = re.match(r'^(\w+)', line.strip())
            if m:
                all_keys.add(m.group(1))
    except struct.error as e:
        errors += 1
        if errors <= 3:
            print(f"Ошибка на записи {i}, pos={pos}: {e}")

print(f"\nВсего уникальных ключей: {len(all_keys)}")
print(f"Ошибок чтения: {errors}")

app_keys = {'Sensitivity', 'Button0', 'Button1', 'Button2', 'Button3', 'Button4', 'Button5', 'Button6', 'Button7', 'Button8', 'Button9', 'Button10', 'Button11', 'Button12', 'Button13', 'Button14', 'Button15', 'Button16', 'DeadzoneX', 'DeadzoneY', 'Mode3Axe', 'FastSpeed', 'Directions', 'Mode', 'TimeoutMove', 'TimeoutSwitchLeftMB', 'FastPush', '2Moves', '2MovesMode1', 'ChangeDirOnTheWay', 'RightMBisKey', 'Alt2', 'Autoclick', 'CircleScale', 'RightMBDoubleClick', 'LeftMBPushTwice', 'RightMBPushTwice', 'DownAll', 'SkipFast', 'UpImmediately', 'CursorVisible', 'MagicWindows', 'NoMoveRightMB', 'AutoclickLMB', 'AutoclickSpeed'}

new_keys = all_keys - app_keys
print('НОВЫЕ параметры в EmbeddedSettings:')
for k in sorted(new_keys):
    print(f'  {k}')