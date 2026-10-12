#!/usr/bin/env python3
# Сверка номеров битов в tests/stub/avr/io.h с настоящим avr-libc (iom16.h).
# Запуск: python3 tests/stub/check_bits.py /путь/к/avr/include/avr/iom16.h
import re, sys, pathlib
stub = pathlib.Path(__file__).resolve().parent / 'avr' / 'io.h'
real = pathlib.Path(sys.argv[1]).read_text(encoding='utf-8', errors='replace')
mine = dict(re.findall(r'^#define\s+(\w+)\s+(\d+)\s*$', stub.read_text(encoding='utf-8'), re.M))
theirs = dict(re.findall(r'^#define\s+(\w+)\s+(\d+)\s*$', real, re.M))
bad = [(k, v, theirs[k]) for k, v in mine.items() if k in theirs and theirs[k] != v]
missing = [k for k in mine if k not in theirs]
print('совпало:', sum(1 for k in mine if k in theirs and theirs[k] == mine[k]), 'расхождений:', len(bad))
for k, v, t in bad: print('  РАСХОДИТСЯ', k, 'stub', v, 'avr-libc', t)
if missing: print('  нет в iom16.h (проверить вручную):', ' '.join(sorted(missing)))
sys.exit(1 if bad else 0)
