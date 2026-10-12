#!/usr/bin/env python3
"""Готовит копии программ для проверки логики на ПК в tests/.work/ (структура как в code/):
  - main -> prog_main (тест вызывает её сам через sim_start() или sim_loop());
  - пустой бесконечный цикл `while(1){ }` (или `for(;;){ }`) -> `return 0;`, непустой цикл остаётся:
    такие программы тест гоняет через sim_loop() (выход по счётчику вызовов _delay_ms);
  - .h копируются как есть;
  - мутанты из tests/mutants/*.json: {"src": "code/...c", "dst": "глава/имя.c", "replace": [["было", "стало"], ...]}
    каждая строка "было" обязана найтись в исходнике, иначе ошибка (мутант устарел).
Запуск: python3 tests/prepare.py"""
import json, pathlib, re, shutil, sys
root = pathlib.Path(__file__).resolve().parent.parent
code, work = root / 'code', root / 'tests' / '.work'
MAIN = re.compile(r'\bint\s+main\s*\(\s*(?:void)?\s*\)')
LOOP = re.compile(r'(?:while\s*\(\s*1\s*\)|for\s*\(\s*;\s*;\s*\))[ \t]*(?://[^\n]*)?\s*\{\s*\}')

def conv(text, name):
    t, n1 = MAIN.subn('int prog_main(void)', text)
    if n1 == 0:
        return text                     # файл без main (часть многофайлового проекта)
    if n1 != 1:
        sys.exit(f'prepare.py: {name}: main() встречается {n1} раз')
    t, n2 = LOOP.subn('return 0;', t)
    if n2 > 1:
        sys.exit(f'prepare.py: {name}: несколько пустых бесконечных циклов')
    return t

for p in work.iterdir() if work.exists() else []:
    if p.name != 'bin':
        shutil.rmtree(p) if p.is_dir() else p.unlink()
work.mkdir(parents=True, exist_ok=True)
n = 0
for src in sorted(code.rglob('*')):
    if src.suffix not in ('.c', '.h') or not src.is_file():
        continue
    dst = work / src.relative_to(code)
    dst.parent.mkdir(parents=True, exist_ok=True)
    text = src.read_text(encoding='utf-8')
    dst.write_text(conv(text, str(src.relative_to(root))) if src.suffix == '.c' else text, encoding='utf-8')
    n += 1
m = 0
for spec in sorted((root / 'tests' / 'mutants').glob('*.json')):
    d = json.loads(spec.read_text(encoding='utf-8'))
    text = (root / d['src']).read_text(encoding='utf-8')
    for old, new in d['replace']:
        if old not in text:
            sys.exit(f'prepare.py: мутант {spec.name}: в {d["src"]} нет строки {old!r}')
        text = text.replace(old, new)
    out = work / d['dst']
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(conv(text, spec.name), encoding='utf-8')
    m += 1
print(f'prepare.py: {n} файлов из code/, {m} мутантов -> {work.relative_to(root)}')
