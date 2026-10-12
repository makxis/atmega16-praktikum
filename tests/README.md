# Проверки курса

Для тех, кто правит курс. Одна команда из корня репозитория (или из любой папки) проверяет всё:

```sh
sh tests/run.sh
```

Нужны avr-gcc, gcc и python3. В Ubuntu и Debian:

```sh
sudo apt-get install gcc-avr avr-libc binutils-avr gcc python3
```

На каждый push и pull request то же самое запускает GitHub Actions ([ci.yml](../.github/workflows/ci.yml)).

## Что именно проверяется

1. **Сборка** ([tests/build.sh](build.sh)). Каждая программа из code/ собирается avr-gcc под ATmega16 двумя наборами флагов: `-Os -Wall -Wextra` и то же с флагами AVR Studio 4 по умолчанию (`-std=gnu99 -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums`). Требование: 0 ошибок и 0 предупреждений, программа влезает во flash (16 КБ) и в RAM (1 КБ).
2. **Логика на ПК** ([tests/logic.sh](logic.sh), модель в [tests/sim.h](sim.h)). Программы собираются обычным gcc, регистры и таймеры подменены моделью: кнопки нажимаются с дребезгом, клавиатура моделируется матрицей, проверяются переносы, полночь, частотомер, ШИМ. Мутанты (копии программ с намеренно внесённой типичной ошибкой, tests/mutants/) показывают, что тест эту ошибку ловит.
3. **Тексты** ([tests/check_docs.py](check_docs.py)). Фрагменты «Пишем вместе» дословно совпадают с полными файлами; ссылки и якоря ведут куда надо; спойлеры оформлены так, что GitHub их отрисует; в программах только ASCII и кириллица (иначе zip в Windows-1251 не соберётся); блок ПОДКЛЮЧЕНИЕ каждой программы укладывается в 16 гнёзд панели.

## Docker, переменные, релиз

Если avr-gcc не установлен, а docker есть, run.sh соберёт всё в контейнерах: alpine:3.20 для avr-gcc, gcc:13 для логики.

| Переменная | Что делает |
|---|---|
| `MODE=native` или `MODE=docker` | выбрать режим явно |
| `AVR_BIN=папка с avr-gcc` | несколько папок через двоеточие: собрать каждым компилятором |
| `CC=gcc` | компилятор для тестов логики |

Полный лог: `sh tests/run.sh 2>&1 | tee run.log`.

Тег вида `v1.0` запускает [release.yml](../.github/workflows/release.yml): проверки, сборка zip в Windows-1251 ([tools/make_zip.py](../tools/make_zip.py)) и релиз с этим zip. Собрать zip вручную: `python3 tools/make_zip.py v1.0`, он появится в папке dist/.

## Структура репозитория

```
README.md                главная страница курса
LICENSE                  лицензия MIT на программы и скрипты (тексты под CC BY 4.0)
docs/                    главы 1–12 и справка
code/NN-тема/            программы по главам: UTF-8, LF, шапка с блоком ПОДКЛЮЧЕНИЕ
tests/README.md          этот файл
tests/run.sh             все проверки одной командой
tests/build.sh           сборка avr-gcc двумя наборами флагов, размеры flash и RAM
tests/prepare.py         копии программ для тестов на ПК (в tests/.work/) и мутанты
tests/logic.sh, sim.h    тесты логики на ПК: модель регистров, таймеров, кнопок,
tests/stub/, logic/        клавиатуры и индикатора; сами тесты в tests/logic/,
tests/mutants/             мутанты в tests/mutants/
tests/check_docs.py      проверки текстов и шапок программ
tools/make_zip.py        zip в Windows-1251 для AVR Studio 4
.github/workflows/       ci.yml: проверки на каждый push; release.yml: zip к релизу по тегу v*
```

[← К курсу](../README.md)
