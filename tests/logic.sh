#!/bin/sh
# Проверка логики программ на ПК (без МК): каждый tests/logic/**/*.c собирается обычным gcc вместе
# с моделью tests/sim.h и копией программы из tests/.work (её готовит tests/prepare.py) и запускается.
# Тест сам печатает "ok имя" или "FAILED имя" и строки "FAIL: ..." с причиной.
# Запуск из корня репозитория: sh tests/logic.sh      (компилятор: CC=gcc по умолчанию)
# SKIP_PREPARE=1: не запускать tests/prepare.py (копии уже готовы, например в контейнере без python3).
cd "$(dirname "$0")/.." || exit 1
CC=${CC:-gcc}
if [ "${SKIP_PREPARE:-0}" != 1 ]; then python3 tests/prepare.py >/dev/null || exit 1; fi
mkdir -p tests/.work/bin
fail=0; n=0; bad=0
for t in $(find tests/logic -name '*.c' | sort); do
  n=$((n + 1))
  name=$(echo "$t" | sed 's#^tests/logic/##; s#\.c$##; s#/#__#g')
  if ! out=$($CC -std=gnu99 -w -I tests/stub -I tests -I tests/.work -o "tests/.work/bin/$name" "$t" -lm 2>&1); then
    echo "FAILED   $t: не собирается"; echo "$out" | head -20; fail=1; bad=$((bad + 1)); continue
  fi
  timeout 120 "tests/.work/bin/$name"; rc=$?
  if [ $rc -ne 0 ]; then
    fail=1; bad=$((bad + 1))
    [ $rc -gt 1 ] && echo "FAILED   $t: код выхода $rc (зависание или падение)"
  fi
done
if [ $fail = 0 ]; then echo "логика: $n тестов, все ok"; else echo "логика: $n тестов, не прошли: $bad"; fi
exit $fail
