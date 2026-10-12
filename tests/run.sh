#!/bin/sh
# Полная проверка курса одной командой (из корня репозитория или из любой папки):
#     sh tests/run.sh
# Шаги:
#   0) python3 tests/prepare.py  копии программ для тестов на ПК (tests/.work), мутанты
#   1) sh tests/build.sh         каждая программа собирается avr-gcc под ATmega16 двумя наборами
#                                флагов: 0 ошибок, 0 предупреждений, влезает во flash и RAM
#   2) sh tests/logic.sh         логика программ на ПК: обычный gcc, регистры подменены (tests/sim.h)
#   3) python3 tests/check_docs.py  фрагменты «Пишем вместе», ссылки и якоря, спойлеры,
#                                кодировка code/, блоки ПОДКЛЮЧЕНИЕ, навигация глав
# Итог: строка «== ИТОГ: всё в порядке» и код 0, иначе «== ИТОГ: есть проблемы» и код 1.
#
# Режимы:
#   нативный (так в GitHub Actions): нужны avr-gcc с avr-size, gcc и python3.
#     Ubuntu/Debian: sudo apt-get install gcc-avr avr-libc binutils-avr gcc python3
#     AVR_BIN=папка с avr-gcc (можно несколько через двоеточие: собрать каждым), CC=gcc
#   docker (если avr-gcc нет, а docker есть): один контейнер alpine:3.20 с apk add gcc-avr
#     на всю сборку, gcc:13 на логику; python3 с хоста, а если его нет, python:3.12-alpine.
#     Первый запуск качает образы и пакеты, нужен интернет.
#   MODE=native или MODE=docker выбирает режим явно.
# Лог целиком:  sh tests/run.sh 2>&1 | tee run.log
ROOT=$(cd "$(dirname "$0")/.." && pwd) || exit 2
cd "$ROOT" || exit 2

have() { command -v "$1" >/dev/null 2>&1; }

# ---- какие инструменты есть
if [ -n "${AVR_BIN:-}" ]; then
  AVR_FIRST=${AVR_BIN%%:*}
  [ -x "$AVR_FIRST/avr-gcc" ] && AVR_OK=1 || AVR_OK=0
else
  have avr-gcc && AVR_OK=1 || AVR_OK=0
fi
if [ -z "${CC:-}" ]; then
  if have gcc; then CC=gcc; elif have cc; then CC=cc; else CC=; fi
fi
PY=; have python3 && PY=python3

MODE=${MODE:-}
if [ -z "$MODE" ]; then
  if [ "$AVR_OK" = 1 ] && [ -n "$CC" ] && [ -n "$PY" ]; then
    MODE=native
  elif have docker && docker info >/dev/null 2>&1; then
    MODE=docker
  else
    echo "Нечем собирать. Нужны либо avr-gcc, gcc и python3 (Ubuntu/Debian:"
    echo "  sudo apt-get install gcc-avr avr-libc binutils-avr gcc python3),"
    echo "либо работающий docker. Нашлось: avr-gcc: $( [ "$AVR_OK" = 1 ] && echo есть || echo нет ),"
    echo "  компилятор Си: ${CC:-нет}, python3: $( [ -n "$PY" ] && echo есть || echo нет ),"
    echo "  docker: $(have docker && echo 'есть, но не отвечает (нет прав или не запущен)' || echo нет)."
    echo "== ИТОГ: есть проблемы (не запущено)"
    exit 2
  fi
fi
case $MODE in native|docker) ;; *) echo "MODE=$MODE: бывает native или docker"; exit 2 ;; esac

UIDGID="$(id -u):$(id -g)"
# python-скрипт из репозитория: python3 хоста, иначе контейнер python:3.12-alpine
run_py() {
  if [ -n "$PY" ]; then
    "$PY" "$@"
  else
    docker run --rm -u "$UIDGID" -v "$ROOT":/w -w /w python:3.12-alpine python3 "$@"
  fi
}

if [ "$MODE" = native ]; then
  if [ -n "${AVR_BIN:-}" ]; then avr_show="$AVR_BIN"; else avr_show=$(command -v avr-gcc); fi
  echo "== режим: нативный"
  echo "   avr-gcc: $avr_show"
  echo "   gcc:     $(command -v "$CC") ($("$CC" -dumpversion 2>/dev/null))"
  echo "   python3: $(command -v "$PY") ($("$PY" -c 'import sys; print(sys.version.split()[0])'))"
else
  echo "== режим: docker (avr-gcc в alpine:3.20, логика в gcc:13, python3: ${PY:-python:3.12-alpine})"
fi

FAIL=""
add_fail() { FAIL="$FAIL${FAIL:+, }$1"; }

echo
echo "== 0. копии программ для тестов (tests/prepare.py)"
if run_py tests/prepare.py; then PREP=1; else PREP=0; add_fail "подготовка"; fi

echo
echo "== 1. сборка avr-gcc под ATmega16 (tests/build.sh)"
if [ "$MODE" = native ]; then
  sh tests/build.sh || add_fail "сборка"
else
  docker run --rm -v "$ROOT":/w -w /w alpine:3.20 sh -c '
    apk add -q gcc-avr binutils-avr avr-libc >/dev/null 2>&1 ||
      { echo "не удалось поставить gcc-avr в alpine:3.20 (нужен интернет)"; exit 2; }
    sh tests/build.sh' || add_fail "сборка"
fi

echo
echo "== 2. логика программ на ПК (tests/logic.sh)"
if [ "$PREP" = 0 ]; then
  echo "пропущено: tests/prepare.py не отработал"
  add_fail "логика"
elif [ "$MODE" = native ]; then
  CC="$CC" SKIP_PREPARE=1 sh tests/logic.sh || add_fail "логика"
else
  # копии программ уже приготовлены на шаге 0 (python3 в образе gcc:13 не нужен)
  docker run --rm -u "$UIDGID" -v "$ROOT":/w -w /w -e CC=gcc -e SKIP_PREPARE=1 gcc:13 \
    sh tests/logic.sh || add_fail "логика"
fi

echo
echo "== 3. тексты, ссылки, шапки программ (tests/check_docs.py)"
run_py tests/check_docs.py || add_fail "тексты"

echo
if [ -z "$FAIL" ]; then
  echo "== ИТОГ: всё в порядке"
  exit 0
fi
echo "== ИТОГ: есть проблемы ($FAIL), подробности выше"
exit 1
