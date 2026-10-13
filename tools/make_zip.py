#!/usr/bin/env python3
"""Архив программ курса для AVR Studio 4: dist/atmega16-praktikum-cp1251.zip.

В репозитории файлы в UTF-8 с переводами строк LF. Редактор AVR Studio 4 читает файлы
в кодировке Windows (cp1251), и русские комментарии из UTF-8 он показывает «кракозябрами».
Поэтому в архив все code/**/*.c и *.h кладутся перекодированными в cp1251 с CRLF,
с той же структурой папок, плюс README.txt (как открыть в Studio 4) и LICENSE.txt.
Если символ не кодируется в cp1251, архив не собирается (ошибка с файлом и строкой).

Запуск:  python3 tools/make_zip.py [версия]
Версия: аргумент (например v1.0); без него `git describe --tags --always --dirty`,
если это git-репозиторий, иначе «без версии». Время файлов в архиве фиксированное
(SOURCE_DATE_EPOCH или время последнего коммита), поэтому одинаковые исходники дают
одинаковый архив.
"""
import os
import pathlib
import subprocess
import sys
import time
import zipfile

ROOT = pathlib.Path(__file__).resolve().parent.parent
OUT = ROOT / 'dist' / 'atmega16-praktikum-cp1251.zip'
TOP = 'atmega16-praktikum'                  # папка внутри архива

README = """\
Си для ATmega16 на стендах Учтех-Профи: программы курса для AVR Studio 4
Версия: {version}

В архиве программы курса в кодировке Windows-1251 с переводами строк CRLF: так
русские комментарии нормально читаются в редакторе AVR Studio 4. Папки те же, что
в папке code/ репозитория: номер в имени папки = номер главы курса. Текст глав и те
же программы в UTF-8 лежат в репозитории курса (папки docs/ и code/).

Что в архиве ({count} файлов):
{listing}

КАК ОТКРЫТЬ ПРОГРАММУ В AVR STUDIO 4
(подробнее, с частыми ошибками компиляции: глава 2 курса «AVR Studio 4: сборка, прошивка, отладка»)

1. Распакуйте архив в папку без русских букв и пробелов в пути, например C:\\AVR\\.
   Studio 4 собирает проект через make, русские буквы и пробелы в пути ломают сборку.
2. Project -> Project Wizard -> New Project.
   Тип проекта: AVR GCC (не Atmel AVR Assembler). Имя проекта латиницей,
   папка, например, C:\\AVR\\. Next.
3. Debug platform: JTAG ICE, Device: ATmega16 -> Finish.
4. Скопируйте нужный файл из архива в папку проекта. В дереве проекта слева правой
   кнопкой по Source Files -> Add Existing Source File(s) и выберите этот файл, а файл,
   созданный мастером, уберите из проекта (Remove File from Project): функция main()
   в проекте должна быть одна. Или проще: откройте файл из архива, выделите весь текст
   (Ctrl+A), скопируйте и вставьте вместо содержимого файла, созданного мастером.
5. Project -> Configuration Options: Frequency = 8000000, Optimization = -Os.
6. Build -> Build (F7). В окне Build не должно быть ни ошибок, ни предупреждений.
7. Прошивка: кнопка Con на панели инструментов -> JTAG ICE, порт Auto (или нужный COM)
   -> Connect -> вкладка Program, секция Flash: файл .hex из папки default проекта
   -> Program.
   Вкладку Fuses НЕ ТРОГАТЬ: так требует инструкция к стенду, неверные фьюзы могут
   заблокировать контроллер.

Перед прошивкой соедините модули проводами по блоку ПОДКЛЮЧЕНИЕ в начале файла
программы: там сказано, что в какое гнездо «Контроллера ПУ» идёт, и сколько гнёзд
из 16 занято.
"""


def version(argv):
    if len(argv) > 1 and argv[1].strip():
        return argv[1].strip()
    try:
        v = subprocess.run(['git', 'describe', '--tags', '--always', '--dirty'], cwd=ROOT,
                           capture_output=True, text=True, timeout=20)
        if v.returncode == 0 and v.stdout.strip():
            return v.stdout.strip()
    except (OSError, subprocess.SubprocessError):
        pass
    return 'без версии'


def stamp():
    """Время файлов в архиве: SOURCE_DATE_EPOCH, иначе время последнего коммита, иначе 2026-01-01."""
    t = os.environ.get('SOURCE_DATE_EPOCH')
    if not t:
        try:
            r = subprocess.run(['git', 'log', '-1', '--format=%ct'], cwd=ROOT,
                               capture_output=True, text=True, timeout=20)
            t = r.stdout.strip() if r.returncode == 0 else ''
        except (OSError, subprocess.SubprocessError):
            t = ''
    if t.isdigit():
        return time.gmtime(int(t))[:6]
    return (2026, 1, 1, 0, 0, 0)


def to_cp1251(text, name, errors):
    """UTF-8 текст -> байты cp1251 с CRLF; неперекодируемые символы в errors."""
    if '\r' in text:
        errors.append(f'{name}: переводы строк CRLF/CR, в репозитории должны быть LF')
        text = text.replace('\r\n', '\n').replace('\r', '\n')
    out = text.replace('\n', '\r\n')
    try:
        return out.encode('cp1251')
    except UnicodeEncodeError:
        for ln, line in enumerate(text.split('\n'), 1):
            for col, ch in enumerate(line, 1):
                try:
                    ch.encode('cp1251')
                except UnicodeEncodeError:
                    errors.append(f'{name}:{ln}:{col}: символ «{ch}» (U+{ord(ch):04X}) нет в Windows-1251')
        return None


def first_comment(text):
    """Первая строка шапки программы (без //): для списка файлов в README.txt."""
    for line in text.split('\n'):
        s = line.strip()
        if s.startswith('//'):
            s = s.lstrip('/').strip()
            if s:
                return s
        elif s:
            break
    return ''


def main(argv):
    ver = version(argv)
    src = sorted(p for p in (ROOT / 'code').rglob('*') if p.is_file() and p.suffix in ('.c', '.h'))
    if not src:
        print('make_zip: в code/ нет файлов .c/.h', file=sys.stderr)
        return 1
    errors, entries, listing = [], [], []
    for p in src:
        name = p.relative_to(ROOT / 'code').as_posix()
        if not all(ord(c) < 128 for c in name) or ' ' in name:
            errors.append(f'code/{name}: в имени файла только латиница без пробелов (Studio 4 и make)')
            continue
        try:
            text = p.read_text(encoding='utf-8')
        except UnicodeDecodeError as e:
            errors.append(f'code/{name}: файл не в UTF-8 (байт {e.start})')
            continue
        data = to_cp1251(text, f'code/{name}', errors)
        if data is not None:
            entries.append((name, data))
            what = first_comment(text) if p.suffix == '.c' else ''
            listing.append(f'  {name}' + (f'\n      {what}' if what else ''))
    lic = ROOT / 'LICENSE'
    if lic.exists():
        data = to_cp1251(lic.read_text(encoding='utf-8'), 'LICENSE', errors)
        if data is not None:
            entries.append(('LICENSE.txt', data))
    readme = README.format(version=ver, count=len(src), listing='\n'.join(listing))
    data = to_cp1251(readme, 'README.txt', errors)
    if data is not None:
        entries.insert(0, ('README.txt', data))
    if errors:
        print('make_zip: архив не собран, исправьте:', file=sys.stderr)
        for e in errors:
            print('  ' + e, file=sys.stderr)
        return 1
    OUT.parent.mkdir(parents=True, exist_ok=True)
    tmp = OUT.with_suffix('.zip.tmp')
    when = stamp()
    with zipfile.ZipFile(tmp, 'w', zipfile.ZIP_DEFLATED, compresslevel=9) as z:
        z.comment = f'atmega16-praktikum {ver}, Windows-1251, CRLF'.encode('ascii', 'replace')
        for name, data in entries:
            info = zipfile.ZipInfo(f'{TOP}/{name}', date_time=when)
            info.compress_type = zipfile.ZIP_DEFLATED
            info.external_attr = 0o644 << 16
            info.create_system = 0                  # «создан в DOS/Windows»: без unix-флагов
            z.writestr(info, data)
    tmp.replace(OUT)
    print(f'make_zip: {OUT.relative_to(ROOT).as_posix()}: {len(src)} файлов из code/ в cp1251 + CRLF, '
          f'README.txt{", LICENSE.txt" if lic.exists() else ""}; '
          f'{"версия " + ver if ver != "без версии" else "без версии"}; {OUT.stat().st_size} байт')
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
