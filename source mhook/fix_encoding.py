#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Скрипт для исправления кодировки и синтаксических ошибок файлов проекта MHook2
Все файлы .cpp и .h должны быть в UTF-8 with BOM (Codepage 65001)
"""

import os
import sys
import argparse
import re
from pathlib import Path

def fix_syntax_errors(content, filepath):
    """
    Исправляет типичные синтаксические ошибки C++
    """
    fixes_applied = []
    
    # Исправление: class ClassName:public BaseClass -> class ClassName : public BaseClass
    # Добавляем пробел перед двоеточием наследования
    pattern_class = re.compile(r'(class\s+\w+):public')
    if pattern_class.search(content):
        content = pattern_class.sub(r'\1 : public', content)
        fixes_applied.append('Добавлен пробел перед :public')
    
    # Исправление: class ClassName:private -> class ClassName : private
    pattern_class_private = re.compile(r'(class\s+\w+):private')
    if pattern_class_private.search(content):
        content = pattern_class_private.sub(r'\1 : private', content)
        fixes_applied.append('Добавлен пробел перед :private')
    
    # Исправление: class ClassName:protected -> class ClassName : protected
    pattern_class_protected = re.compile(r'(class\s+\w+):protected')
    if pattern_class_protected.search(content):
        content = pattern_class_protected.sub(r'\1 : protected', content)
        fixes_applied.append('Добавлен пробел перед :protected')
    
    return content, fixes_applied

def fix_line_endings(content, filepath):
    """
    Приводит все окончания строк к CRLF (стандарт Windows/Visual Studio)
    Удаляет ВСЕ пустые строки
    """
    # Skip resource.h - it needs empty lines
    filepath_str = str(filepath)
    if 'resource.h' in filepath_str:
        return content, True
    
    # First normalize to \n
    content = content.replace('\r\n', '\n').replace('\r', '\n')
    lines = content.split('\n')
    # Remove all empty lines
    result = [line.rstrip() for line in lines if line.strip()]
    normalized = '\r\n'.join(result)
    return normalized, True

def fix_file_encoding(filepath):
    """
    Исправляет кодировку файла на UTF-8 with BOM и синтаксические ошибки
    """
    try:
        # Читаем файл в бинарном режиме
        with open(filepath, 'rb') as f:
            raw_content = f.read()
        
        # Проверяем BOM
        has_utf8_bom = raw_content.startswith(b'\xef\xbb\xbf')
        has_utf16_le_bom = raw_content.startswith(b'\xff\xfe')
        has_utf16_be_bom = raw_content.startswith(b'\xfe\xff')
        
        # Если уже UTF-8 with BOM - пропускаем
        if has_utf8_bom:
            # Читаем в binary режиме чтобы не конвертировать окончания строк
            content = raw_content[3:].decode('utf-8', errors='replace')
            
            # Проверяем и исправляем синтаксис
            content, syntax_fixes = fix_syntax_errors(content, filepath)
            
            # Приводим окончания строк к CRLF (стандарт Windows/Visual Studio)
            content, line_ending_fixed = fix_line_endings(content, filepath)
            
            if line_ending_fixed:
                print(f"[LINEEND] {filepath} - окончания строк приведены к CRLF")
            
            # Записываем с UTF-8 with BOM (используем binary mode для правильного BOM)
            with open(filepath, 'wb') as f:
                # Пишем BOM bytes
                f.write(b'\xef\xbb\xbf')
                # Пишем контент как UTF-8
                f.write(content.encode('utf-8'))
            
            return True
        
        # Определяем текущую кодировку и декодируем
        content = None
        
        if has_utf16_le_bom:
            # UTF-16 LE with BOM
            try:
                content = raw_content.decode('utf-16-le')
                print(f"[CONV] {filepath} - конвертация UTF-16 LE -> UTF-8 with BOM")
            except:
                pass
        elif has_utf16_be_bom:
            # UTF-16 BE with BOM
            try:
                content = raw_content.decode('utf-16-be')
                print(f"[CONV] {filepath} - конвертация UTF-16 BE -> UTF-8 with BOM")
            except:
                pass
        else:
            # Пробуем разные кодировки
            encodings = ['utf-8', 'windows-1251', 'cp1251', 'latin-1', 'iso-8859-1']
            for encoding in encodings:
                try:
                    content = raw_content.decode(encoding)
                    if encoding != 'utf-8':
                        print(f"[CONV] {filepath} - конвертация {encoding} -> UTF-8 with BOM")
                    else:
                        print(f"[CONV] {filepath} - добавление BOM к UTF-8")
                    break
                except:
                    continue
        
        if content is None:
            print(f"[ERROR] {filepath} - не удалось определить кодировку")
            return False
        
        # Исправляем синтаксические ошибки
        content, syntax_fixes = fix_syntax_errors(content, filepath)
        
        # Приводим окончания строк к CRLF (стандарт Windows/Visual Studio)
        content, line_ending_fixed = fix_line_endings(content, filepath)
        
        if line_ending_fixed:
            print(f"[LINEEND] {filepath} - окончания строк приведены к CRLF")
        
        # Записываем с UTF-8 with BOM (используем binary mode для правильного BOM)
        with open(filepath, 'wb') as f:
            # Пишем BOM bytes
            f.write(b'\xef\xbb\xbf')
            # Пишем контент как UTF-8
            f.write(content.encode('utf-8'))
        
        if syntax_fixes:
            print(f"[SYNTAX] {filepath}: {', '.join(syntax_fixes)}")
        
        print(f"[FIXED] {filepath} - исправлено")
        return True
        
    except Exception as e:
        print(f"[ERROR] {filepath} - ошибка: {e}")
        return False

def process_directory(directory, extensions=None):
    """
    Обрабатывает все файлы в директории с указанными расширениями
    """
    if extensions is None:
        extensions = ['.cpp', '.h', '.hpp']
    
    directory = Path(directory)
    fixed_count = 0
    error_count = 0
    syntax_count = 0
    
    print(f"\n[SCAN] Поиск файлов в: {directory}")
    print(f"[EXT] Расширения: {', '.join(extensions)}\n")
    
    for ext in extensions:
        for filepath in directory.rglob(f'*{ext}'):
            # Пропускаем временные файлы и x64
            if any(skip in str(filepath) for skip in ['temp', 'x64', 'Debug', 'Release']):
                continue
                
            if fix_file_encoding(filepath):
                fixed_count += 1
            else:
                error_count += 1
    
    print(f"\n[STATS] Статистика:")
    print(f"   [FIXED] Исправлено: {fixed_count}")
    print(f"   [ERROR] Ошибок: {error_count}")
    
    return error_count == 0

def main():
    parser = argparse.ArgumentParser(
        description='Исправление кодировки и синтаксических ошибок файлов MHook2'
    )
    parser.add_argument(
        'path',
        nargs='?',
        default='.',
        help='Путь к директории или файлу (по умолчанию: текущая директория)'
    )
    parser.add_argument(
        '--extensions',
        '-e',
        nargs='+',
        default=['.cpp', '.h', '.hpp'],
        help='Расширения файлов для обработки (по умолчанию: .cpp .h .hpp)'
    )
    
    args = parser.parse_args()
    
    path = Path(args.path)
    
    if not path.exists():
        print(f"[ERROR] Ошибка: путь не существует: {path}")
        sys.exit(1)
    
    if path.is_file():
        # Обрабатываем один файл
        success = fix_file_encoding(path)
        sys.exit(0 if success else 1)
    else:
        # Обрабатываем директорию
        success = process_directory(path, args.extensions)
        sys.exit(0 if success else 1)

if __name__ == '__main__':
    main()
