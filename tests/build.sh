#!/bin/sh
# Сборка всех программ курса avr-gcc под ATmega16. Требование: 0 ошибок и 0 предупреждений,
# любой вывод компилятора считается провалом.
#   - каждая программа из code/ (файл .c с main) собирается двумя наборами флагов:
#       1) -Os -Wall -Wextra                    (как в командной строке)
#       2) то же + флаги AVR Studio 4 по умолчанию (-std=gnu99 -funsigned-char ...);
#   - проект из нескольких файлов (папка, где рядом с main.c лежат другие .c) собирается
#     целиком, всеми .c сразу, этими же наборами флагов;
#   - avr-size: flash (text+data) не больше 16384 байт, RAM (data+bss) не больше 1024 байт
#     (стек сверх этого, поэтому запас нужен).
# Запуск из любой папки: sh tests/build.sh
# Компилятор: avr-gcc из PATH или из папки AVR_BIN; можно несколько папок через двоеточие,
#   тогда всё собирается каждым компилятором по очереди:
#   AVR_BIN=/opt/avr-gcc-14/bin:/opt/avr-gcc-7.3/bin sh tests/build.sh
ROOT=$(cd "$(dirname "$0")/.." && pwd) || exit 2
cd "$ROOT" || exit 2

FLAGS1="-Os -Wall -Wextra"
FLAGS2="-Os -Wall -Wextra -std=gnu99 -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums"
FLASH_MAX=16384
RAM_MAX=1024

# русские окончания: plural 5 программа программы программ -> "программ"
plural() {
  n=$1; n100=$((n % 100)); n10=$((n % 10))
  if [ "$n100" -ge 11 ] && [ "$n100" -le 14 ]; then echo "$4"
  elif [ "$n10" -eq 1 ]; then echo "$2"
  elif [ "$n10" -ge 2 ] && [ "$n10" -le 4 ]; then echo "$3"
  else echo "$4"; fi
}

TMP=$(mktemp -d 2>/dev/null || echo "${TMPDIR:-/tmp}/build.$$")
mkdir -p "$TMP" || exit 2
trap 'rm -rf "$TMP"' EXIT
trap 'exit 130' INT TERM

# ---- что собирать
if find code -name '*[[:space:]]*' | grep -q .; then
  echo "в именах файлов или папок code/ есть пробелы: переименуйте"; exit 2
fi
: > "$TMP/progs"                     # строки: "имя|файлы через пробел|1 если проект из нескольких файлов"
: > "$TMP/fragments"
projects=""
for m in $(find code -type f -name main.c | LC_ALL=C sort); do
  d=$(dirname "$m")
  if [ "$(find "$d" -maxdepth 1 -type f -name '*.c' | wc -l)" -gt 1 ]; then
    projects="$projects $d "
    files=$(find "$d" -maxdepth 1 -type f -name '*.c' | LC_ALL=C sort | tr '\n' ' ')
    echo "$d/|${files% }|1" >> "$TMP/progs"
  fi
done
for f in $(find code -type f -name '*.c' | LC_ALL=C sort); do
  case "$projects" in *" $(dirname "$f") "*) continue ;; esac
  if grep -Eq '^[[:space:]]*(int|void)[[:space:]]+main[[:space:]]*\(' "$f"; then
    echo "$f|$f|0" >> "$TMP/progs"
  else
    echo "$f" >> "$TMP/fragments"
  fi
done
LC_ALL=C sort -t'|' -k1,1 "$TMP/progs" > "$TMP/progs.sorted" && mv "$TMP/progs.sorted" "$TMP/progs"
NPROG=$(wc -l < "$TMP/progs" | tr -d ' ')
if [ "$NPROG" -eq 0 ]; then echo "в code/ нет ни одной программы"; exit 2; fi

# ---- компиляторы
if [ -n "${AVR_BIN:-}" ]; then
  dirs=$(echo "$AVR_BIN" | tr ':' ' ')
else
  g=$(command -v avr-gcc 2>/dev/null) || { echo "avr-gcc не найден: поставьте gcc-avr, avr-libc, binutils-avr или задайте AVR_BIN=папка"; exit 2; }
  dirs=$(dirname "$g")
fi

total_err=0; total_warn=0; total_size=0; ran=0
for dir in $dirs; do
  CCAVR="$dir/avr-gcc"; SIZE="$dir/avr-size"
  [ -x "$CCAVR" ] || { echo "нет $CCAVR"; total_err=$((total_err + 1)); continue; }
  [ -x "$SIZE" ] || { echo "нет $SIZE (пакет binutils-avr)"; total_err=$((total_err + 1)); continue; }
  ran=$((ran + 1))
  echo "компилятор: $("$CCAVR" --version | head -n 1)  ($CCAVR)"
  echo "  набор 1: $FLAGS1"
  echo "  набор 2: $FLAGS2  (как в AVR Studio 4)"
  err=0; warn=0; sizebad=0
  : > "$TMP/sizes"
  while IFS='|' read -r name files multi; do
    row="$name"
    for s in 1 2; do
      case $s in 1) flags=$FLAGS1 ;; 2) flags=$FLAGS2 ;; esac
      rm -f "$TMP/x.elf"
      # shellcheck disable=SC2086  # флаги и список файлов разбиваются на слова намеренно
      out=$("$CCAVR" -mmcu=atmega16 $flags -o "$TMP/x.elf" $files 2>&1); rc=$?
      ne=$(printf '%s\n' "$out" | grep -v 'collect2: error: ld returned' | grep -c -E 'error:|undefined reference')
      nw=$(printf '%s\n' "$out" | grep -c 'warning:')
      [ "$rc" -ne 0 ] && [ "$ne" -eq 0 ] && ne=1
      [ -n "$out" ] && [ "$ne" -eq 0 ] && [ "$nw" -eq 0 ] && nw=1     # любой вывод = провал
      if [ -n "$out" ] || [ "$rc" -ne 0 ]; then
        echo "--- $name (набор $s): ошибок $ne, предупреждений $nw"
        printf '%s\n' "$out" | sed "s#$ROOT/##g; s#$TMP/##g" | head -n 40
        err=$((err + ne)); warn=$((warn + nw))
      fi
      if [ "$rc" -eq 0 ] && [ -f "$TMP/x.elf" ]; then
        # avr-size (формат Berkeley): text data bss dec hex filename
        sz=$("$SIZE" "$TMP/x.elf" 2>/dev/null | awk 'NR == 2 && $1 ~ /^[0-9]+$/ { print $1, $2, $3 }')
        if [ -z "$sz" ]; then
          echo "--- $name (набор $s): avr-size не прочитал $TMP/x.elf"
          sizebad=$((sizebad + 1)); row="$row ? ?"; continue
        fi
        # shellcheck disable=SC2086  # три числа разбиваются на $1 $2 $3 намеренно
        set -- $sz
        flash=$(($1 + $2)); ram=$(($2 + $3))
        row="$row $flash $ram"
        if [ "$flash" -gt "$FLASH_MAX" ] || [ "$ram" -gt "$RAM_MAX" ]; then
          echo "--- $name (набор $s): не влезает в ATmega16: flash $flash из $FLASH_MAX, RAM $ram из $RAM_MAX"
          sizebad=$((sizebad + 1))
        fi
      else
        row="$row - -"
      fi
    done
    echo "$row" >> "$TMP/sizes"
  done < "$TMP/progs"
  echo "размеры, байт (flash = text+data из $FLASH_MAX, RAM = data+bss из $RAM_MAX без стека):"
  awk 'function pad(n) { return sprintf("%" n "s", "") }
       BEGIN { printf "  программа%s %8s %6s %8s %6s\n", pad(25), "flash 1", "RAM 1", "flash 2", "RAM 2" }
       { printf "  %-34s %8s %6s %8s %6s\n", $1, $2, $3, $4, $5 }' "$TMP/sizes"
  if [ -s "$TMP/fragments" ]; then
    echo "без main(), отдельно не собираются (фрагменты для вставки по тексту главы): $(tr '\n' ' ' < "$TMP/fragments")"
  fi
  nmulti=$(grep -c '|1$' "$TMP/progs")
  if [ "$nmulti" -gt 0 ]; then
    echo "проекты из нескольких файлов: $(grep '|1$' "$TMP/progs" | cut -d'|' -f1 | tr '\n' ' ')(все .c вместе)"
  fi
  [ "$sizebad" -gt 0 ] && echo "НЕ ВЛЕЗАЮТ В ПАМЯТЬ: $sizebad сборок"
  echo "$NPROG $(plural "$NPROG" программа программы программ) × 2 набора флагов: $err $(plural "$err" ошибка ошибки ошибок), $warn $(plural "$warn" предупреждение предупреждения предупреждений)"
  total_err=$((total_err + err)); total_warn=$((total_warn + warn)); total_size=$((total_size + sizebad))
done

[ "$ran" -gt 0 ] || { echo "не запущено ни одного компилятора"; exit 2; }
[ $((total_err + total_warn + total_size)) -eq 0 ]
