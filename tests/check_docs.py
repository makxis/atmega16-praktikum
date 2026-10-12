#!/usr/bin/env python3
r"""Проверка текстов курса и шапок программ. Только стандартная библиотека Python 3.

Запуск из любой папки:  python3 tests/check_docs.py
Ошибка любой проверки даёт код возврата 1, предупреждения проверку не валят.

а) «Пишем вместе»: строка <!-- фрагмент: ПУТЬ --> стоит прямо перед блоком кода (ПУТЬ
   относительно .md). Каждая непустая строка блока, кроме строк, которые после ведущих
   пробелов начинаются с //, и строк с /*, дословно (с ведущими пробелами, без хвостовых)
   есть в файле ПУТЬ.
б) Относительные ссылки в README.md и docs/*.md ведут на существующие файлы и папки
   репозитория, якоря существуют. Якорь считается так же, как на GitHub: текст заголовка
   строчными буквами, всё, кроме букв, цифр, «_», «-» и пробела, удаляется, пробел -> «-»,
   повторы получают -1, -2 ... README ссылается на каждую главу и каждую папку code/NN-*.
в) <details>: после строки с </summary> пустая строка, перед </details> пустая строка,
   каждый <details> закрыт (иначе GitHub не отрисует markdown внутри спойлера).
г) code/**/*.c|h: только ASCII и кириллица (А-Я, а-я, Ё, ё), без табуляции, переводы строк LF.
   Иначе zip для AVR Studio 4 (Windows-1251) не соберётся.
д) Шапка каждой программы (у проекта из нескольких .c: main.c) содержит блок ПОДКЛЮЧЕНИЕ:
   строки «что -> гнёзда (уровень)», гнёзда вида PD0..PD7 или PB5, PB6; все гнёзда из 16
   выведенных на панель (PC0, PC1, PB2-PB7, PD0-PD7); различных гнёзд ровно N из
   последней строки блока «гнёзд занято: N из 16».
е) Запрещённое. В README и docs: дни недели, «пара N», «на паре», «пары» (кроме «пары проводов»
   и т. п.), «выходные» (кроме «выходной сигнал, уровень ...»), имена людей из исходников, пути
   с именем пользователя, даты. В code: имена, пути с именем пользователя, даты, дни недели.
ж) Каждая глава начинается с H1 из таблицы брифа и заканчивается строкой навигации;
   обязательные заголовки (на них ссылаются другие главы) на месте.
з) Длинное тире «—»: сколько в каждом файле (предупреждение, не ошибка).
и) Таблицы: в строке таблицы (и в любой строке, которая начинается с «|») нет неэкранированного
   «|» внутри `кода`: GitHub режет по нему ячейку, остаток строки пропадает, писать надо «\|».
   В каждой строке не больше ячеек, чем в заголовке (лишние GitHub молча выбрасывает).
к) Каждую программу из code/ подключает хотя бы один тест tests/logic/**/*.c (предупреждение).
"""
import difflib
import html
import pathlib
import re
import sys
import unicodedata
import urllib.parse

sys.dont_write_bytecode = True
ROOT = pathlib.Path(__file__).resolve().parent.parent
QUIET = '--quiet' in sys.argv          # печатать только итоги проверок, без списков

# ---------------------------------------------------------------- главы и заголовки (бриф, раздел 1)
CHAPTERS = [
    ('docs/01-oborudovanie.md', '1. Оборудование'),
    ('docs/02-avr-studio.md', '2. AVR Studio 4: сборка, прошивка, отладка'),
    ('docs/03-si.md', '3. Си для микроконтроллера'),
    ('docs/04-porty.md', '4. Порты, светодиоды, кнопки'),
    ('docs/05-indikator.md', '5. Семисегментный индикатор'),
    ('docs/06-preryvaniya.md', '6. Прерывания'),
    ('docs/07-tajmery.md', '7. Таймеры-счётчики и ШИМ'),
    ('docs/08-chasy.md', '8. Часы MM:SS'),
    ('docs/09-knopka.md', '9. Счётчик нажатий и дребезг'),
    ('docs/10-klaviatura.md', '10. Матричная клавиатура 4×4'),
    ('docs/11-signaly.md', '11. Внешние сигналы и ШИМ'),
    ('docs/12-ne-rabotaet.md', '12. Не работает: что проверить'),
    ('docs/spravka.md', 'Справка'),
]
REQUIRED = {
    'docs/01-oborudovanie.md': ['## Панель «Контроллера ПУ»', '## 16 гнёзд: считаем перед подключением',
                                '## Общий провод', '## Фьюзы не трогать'],
    'docs/02-avr-studio.md': ['## Новый проект', '## Прошивка через JTAG ICE', '## Отладка по шагам',
                              '## Кодировка Windows-1251', '## Частые ошибки компиляции'],
    'docs/03-si.md': ['## Типы данных на AVR', '## Битовые операции', '## volatile и атомарность'],
    'docs/04-porty.md': ['## Регистры DDR, PORT, PIN', '## Подтяжка и бит PUD'],
    'docs/05-indikator.md': ['## Коды цифр', '## Динамическая индикация', '### Шаг 2. Какой разряд где'],
    'docs/06-preryvaniya.md': ['## Три условия', '## Векторы ATmega16', '## Внешние прерывания INT0, INT1, INT2'],
    'docs/07-tajmery.md': ['## Формулы', '## Выводы таймеров на панели', '## ШИМ', '## Timer2 и асинхронный режим'],
    'docs/09-knopka.md': ['## Дребезг'],
    'docs/10-klaviatura.md': ['## Почему 4 разряда и клавиатура не влезают'],
    'docs/spravka.md': ['## Регистры', '## Коды сегментов', '## Таймерные числа при 8 МГц'],
}
SELFCHECK = 'Вопросы для самопроверки'

# гнёзда на панели «Контроллера ПУ»
PANEL = {'PC0', 'PC1'} | {f'PB{i}' for i in range(2, 8)} | {f'PD{i}' for i in range(8)}


# ---------------------------------------------------------------- отчёт
class Report:
    def __init__(self):
        self.sections = []                   # (название, итог, ошибки, предупреждения)
        self.cur = None

    def start(self, title):
        self.cur = [title, '', [], []]
        self.sections.append(self.cur)

    def err(self, where, msg):
        self.cur[2].append(f'{where}: {msg}')

    def warn(self, where, msg):
        self.cur[3].append(f'{where}: {msg}')

    def summary(self, text):
        self.cur[1] = text


R = Report()


def rel(p):
    return pathlib.Path(p).resolve().relative_to(ROOT).as_posix()


def read(p):
    try:
        return pathlib.Path(p).read_text(encoding='utf-8')
    except UnicodeDecodeError as e:
        R.err(rel(p), f'файл не в UTF-8 ({e.reason}, байт {e.start})')
        return pathlib.Path(p).read_bytes().decode('utf-8', 'replace')


# ---------------------------------------------------------------- разбор markdown
FENCE = re.compile(r'^(\s*)(`{3,}|~{3,})(.*)$')
ATX = re.compile(r'^ {0,3}(#{1,6})(?:[ \t]+(.*?))?(?:[ \t]+#+)?[ \t]*$')
SETEXT = re.compile(r'^ {0,3}(=+|-+)[ \t]*$')
CODESPAN = re.compile(r'(`+)(.+?)(?<!`)\1(?!`)')
MARKER = re.compile(r'^\s*<!--\s*фрагмент:\s*(\S+)\s*-->\s*$')


def slug_text(s):
    """Якорь заголовка, как его делает GitHub (буквы, метки, цифры, «_», «-», пробел -> «-»)."""
    out = []
    for ch in s.lower():
        cat = unicodedata.category(ch)
        if ch in ' -' or cat[0] in 'LM' or cat in ('Nd', 'Pc'):
            out.append(ch)
    return ''.join(out).replace(' ', '-')


def heading_plain(raw):
    """Текст заголовка без разметки (то, что видно на странице)."""
    spans = []

    def keep(m):
        spans.append(m.group(2).strip() if m.group(2).strip() else m.group(2))
        return f'\x00{len(spans) - 1}\x00'
    t = CODESPAN.sub(keep, raw.strip())
    t = re.sub(r'!\[[^\]]*\]\([^)]*\)', '', t)                  # картинки
    t = re.sub(r'\[([^\]]*)\]\([^)]*\)', r'\1', t)              # [текст](адрес)
    t = re.sub(r'\[([^\]]*)\]\[[^\]]*\]', r'\1', t)             # [текст][метка]
    t = re.sub(r'</?[A-Za-z][^<>]*>', '', t)                    # html-теги
    t = re.sub(r'\\([!-/:-@\[-`{-~])', r'\1', t)                # \* \_ ...
    t = re.sub(r'(?<!\w)(_{1,2})(?=\S)(.+?)(?<=\S)\1(?!\w)', r'\2', t)   # _курсив_
    t = html.unescape(t)
    return re.sub(r'\x00(\d+)\x00', lambda m: spans[int(m.group(1))], t).strip()


class Md:
    """Строки markdown-файла: где блоки кода, заголовки, якоря."""
    cache = {}

    @classmethod
    def get(cls, path):
        path = pathlib.Path(path).resolve()
        if path not in cls.cache:
            cls.cache[path] = Md(path)
        return cls.cache[path]

    def __init__(self, path):
        self.path = path
        self.rel = rel(path)
        self.lines = read(path).split('\n')
        n = len(self.lines)
        self.code = [False] * n          # строка внутри ``` ``` (включая сами ```)
        self.comment = [False] * n       # строка внутри многострочного <!-- -->
        self.blocks = []                 # (строка ```, строка закрывающей ```, отступ, язык)
        self.unclosed = None
        opened = None
        in_comment = False
        for i, line in enumerate(self.lines):
            if opened is not None:
                self.code[i] = True
                m = FENCE.match(line)
                if m and m.group(2)[0] == opened[2][0] and len(m.group(2)) >= len(opened[2]) \
                        and not m.group(3).strip():
                    self.blocks.append((opened[0], i, opened[1], opened[3]))
                    opened = None
                continue
            if in_comment:
                self.comment[i] = True
                if '-->' in line:
                    in_comment = False
                continue
            m = FENCE.match(line)
            if m and not (m.group(2)[0] == '`' and '`' in m.group(3)):
                opened = (i, len(m.group(1)), m.group(2), m.group(3).strip())
                self.code[i] = True
                continue
            tail = re.sub(r'<!--.*?-->', '', line)
            if '<!--' in tail:
                in_comment = True
        if opened is not None:
            self.unclosed = opened[0]
            self.blocks.append((opened[0], n, opened[1], opened[3]))
        self._headings = None

    def text_lines(self):
        """(номер, строка) вне блоков кода и комментариев; `код` и <!-- --> вырезаны."""
        for i, line in enumerate(self.lines):
            if self.code[i] or self.comment[i]:
                continue
            line = re.sub(r'<!--.*?-->', '', line)
            line = re.sub(r'<!--.*$', '', line)
            line = CODESPAN.sub(lambda m: ' ' * len(m.group(0)), line)
            yield i, line

    def headings(self):
        """[(номер строки, уровень, исходная строка, текст, якорь)]"""
        if self._headings is not None:
            return self._headings
        res, seen = [], {}
        prev_blank = True
        for i, line in enumerate(self.lines):
            if self.code[i] or self.comment[i]:
                prev_blank = False
                continue
            m = ATX.match(line)
            level = None
            if m:
                level, raw = len(m.group(1)), (m.group(2) or '')
            elif (i + 1 < len(self.lines) and not self.code[i + 1] and line.strip() and prev_blank
                  and SETEXT.match(self.lines[i + 1])
                  and not re.match(r'^\s*([-*+>|<]|\d+[.)]\s)', line)):
                level, raw = (1 if self.lines[i + 1].strip()[0] == '=' else 2), line
            if level:
                text = heading_plain(raw)
                base = slug_text(text)
                if base in seen:
                    seen[base] += 1
                    anchor = f'{base}-{seen[base]}'
                else:
                    seen[base] = 0
                    anchor = base
                res.append((i, level, line.rstrip(), text, anchor))
            prev_blank = not line.strip()
        self._headings = res
        return res

    def anchors(self):
        a = {h[4] for h in self.headings()}
        for _, line in self.text_lines():
            for m in re.finditer(r'<[A-Za-z][^>]*\s(?:id|name)\s*=\s*["\']([^"\']+)["\']', line):
                a.add(m.group(1))
        return a


# якоря обязательных заголовков: на них можно ссылаться из других глав
REQUIRED_ANCHORS = {f: {slug_text(heading_plain(h.lstrip('#'))) for h in hs} for f, hs in REQUIRED.items()}
for _f, _ in CHAPTERS:
    REQUIRED_ANCHORS.setdefault(_f, set())


def md_files():
    files = [ROOT / 'README.md'] if (ROOT / 'README.md').exists() else []
    files += sorted((ROOT / 'docs').glob('*.md'))
    return files


def code_files():
    return sorted(p for p in (ROOT / 'code').rglob('*') if p.is_file() and p.suffix in ('.c', '.h'))


def strip_c_comments(t):
    t = re.sub(r'/\*.*?\*/', lambda m: re.sub(r'[^\n]', ' ', m.group(0)), t, flags=re.S)
    return re.sub(r'//[^\n]*', '', t)


def has_main(text):
    return re.search(r'^\s*(?:int|void)\s+main\s*\(', strip_c_comments(text), re.M) is not None


def programs():
    """[(главный файл, [все .c программы])]: одиночные .c с main и проекты из нескольких .c."""
    projects = {}
    for m in (ROOT / 'code').rglob('main.c'):
        cs = sorted(m.parent.glob('*.c'))
        if len(cs) > 1:
            projects[m.parent] = (m, cs)
    res, fragments = [], []
    for p in code_files():
        if p.suffix != '.c':
            continue
        if p.parent in projects:
            if p.name == 'main.c':
                res.append(projects[p.parent])
            continue
        if has_main(read(p)):
            res.append((p, [p]))
        else:
            fragments.append(p)
    return res, fragments


# ---------------------------------------------------------------- а) фрагменты «Пишем вместе»
def check_fragments():
    R.start('фрагменты «Пишем вместе»')
    n_markers = n_lines = 0
    for f in md_files():
        md = Md.get(f)
        opens = {b[0]: b for b in md.blocks}
        cur_heading = ''
        heads = {h[0]: h[2] for h in md.headings()}
        marked = set()
        for i, line in enumerate(md.lines):
            if md.code[i] or md.comment[i]:
                continue
            if '<!--' in line and 'фрагмент' in line:
                m = MARKER.match(line)
                if not m:
                    R.err(f'{md.rel}:{i + 1}', f'маркер не по формату «<!-- фрагмент: ПУТЬ -->»: {line.strip()}')
                    continue
                n_markers += 1
                target = (f.parent / urllib.parse.unquote(m.group(1))).resolve()
                if i + 1 not in opens:
                    R.err(f'{md.rel}:{i + 1}', 'после маркера фрагмента сразу (без пустой строки) должен идти блок ```')
                    continue
                marked.add(i + 1)
                if not target.is_file():
                    R.err(f'{md.rel}:{i + 1}', f'файла фрагмента нет: {m.group(1)}')
                    continue
                full = {l.rstrip() for l in read(target).split('\n')}
                start, end, indent, _ = opens[i + 1]
                for k in range(start + 1, min(end, len(md.lines))):
                    s = md.lines[k]
                    cut = len(s) - len(s.lstrip(' '))
                    s = s[min(cut, indent):].rstrip()
                    if not s.strip() or s.lstrip().startswith('//') or '/*' in s:
                        continue
                    n_lines += 1
                    if s not in full:
                        near = difflib.get_close_matches(s, list(full), n=1, cutoff=0.75)
                        hint = f' (похожая в файле: {near[0].strip()!r})' if near else ''
                        R.err(f'{md.rel}:{k + 1}', f'строки нет в {rel(target)}: {s.strip()!r}{hint}')
        # блоки кода в шагах «Пишем вместе» без маркера: только предупреждение
        for start, end, indent, lang in md.blocks:
            h = ''
            for hi in sorted(heads):
                if hi < start:
                    h = heads[hi]
            if re.match(r'^#{2,4}\s+Шаг\s+\d+', h) and start not in marked and lang in ('c', 'C', ''):
                R.warn(f'{md.rel}:{start + 1}', f'блок кода в «{h.lstrip("# ")}» без маркера <!-- фрагмент: ... -->')
    R.summary(f'{n_markers} маркеров, {n_lines} строк кода сверено с полными файлами')


# ---------------------------------------------------------------- б) ссылки и якоря
LINK_INLINE = re.compile(r'(!?)\[((?:[^\[\]]|\[[^\[\]]*\])*)\]\(\s*(<[^>]*>|[^\s()]*(?:\([^\s()]*\)[^\s()]*)*)'
                         r'(?:\s+(?:"[^"]*"|\'[^\']*\'|\([^)]*\)))?\s*\)')
LINK_REFDEF = re.compile(r'^ {0,3}\[([^\]]+)\]:\s*(<[^>]*>|\S+)')
LINK_HTML = re.compile(r'<(?:a|img|source)\s[^>]*?(?:href|src)\s*=\s*["\']([^"\']*)["\']', re.I)


def links_of(md):
    for i, line in md.text_lines():
        for m in LINK_INLINE.finditer(line):
            yield i, m.group(3)
        m = LINK_REFDEF.match(line)
        if m:
            yield i, m.group(2)
        for m in LINK_HTML.finditer(line):
            yield i, m.group(1)


def check_links():
    R.start('ссылки и якоря')
    n_rel = n_ext = n_anchor = 0
    linked = {}                       # README: на что ссылается
    for f in md_files():
        md = Md.get(f)
        for i, url in links_of(md):
            where = f'{md.rel}:{i + 1}'
            url = url.strip()
            if url.startswith('<') and url.endswith('>'):
                url = url[1:-1].strip()
            if not url:
                R.err(where, 'пустая ссылка ()')
                continue
            if re.match(r'^[A-Za-z][A-Za-z0-9+.\-]*:', url) or url.startswith('//'):
                n_ext += 1
                continue
            n_rel += 1
            path, _, frag = url.partition('#')
            path = urllib.parse.unquote(path.split('?')[0])
            frag = urllib.parse.unquote(frag)
            if not path:
                target = f.resolve()
            elif path.startswith('/'):
                target = (ROOT / path.lstrip('/')).resolve()
            else:
                target = (f.parent / path).resolve()
            if target != ROOT and ROOT not in target.parents:
                R.err(where, f'ссылка ведёт за пределы репозитория: {url}')
                continue
            if not target.exists():
                R.err(where, f'нет такого файла или папки: {url}')
                continue
            if target.is_dir() and not any(x.is_file() for x in target.rglob('*')):
                R.err(where, f'папка пустая: git её не сохранит, на GitHub ссылка будет битой: {url}')
                continue
            if md.rel == 'README.md':
                linked[target] = True
            if not frag:
                continue
            n_anchor += 1
            if target.is_dir():
                R.err(where, f'якорь у ссылки на папку: {url}')
            elif target.suffix == '.md':
                anchors = Md.get(target).anchors()
                if frag not in anchors:
                    low = [a for a in anchors if a.lower() == frag.lower()]
                    near = low or difflib.get_close_matches(frag, sorted(anchors), n=1, cutoff=0.6)
                    hint = f' (есть «#{near[0]}»)' if near else ''
                    R.err(where, f'в {rel(target)} нет якоря «#{frag}»{hint}')
                elif target != f.resolve() and rel(target) in REQUIRED_ANCHORS \
                        and frag not in REQUIRED_ANCHORS[rel(target)]:
                    R.warn(where, f'якорь «#{frag}» главы {rel(target)} не из таблицы брифа: '
                           'при правке той главы может пропасть (надёжнее ссылка на файл главы)')
            else:
                m = re.fullmatch(r'L(\d+)(?:-L(\d+))?', frag)
                if m:
                    nlines = len(read(target).split('\n'))
                    if int(m.group(2) or m.group(1)) > nlines:
                        R.err(where, f'в {rel(target)} всего {nlines} строк: {url}')
                else:
                    R.warn(where, f'якорь «#{frag}» у файла {rel(target)} не проверяется')
    # README ссылается на все главы и все папки кода
    if (ROOT / 'README.md').exists():
        for f, _ in CHAPTERS:
            p = (ROOT / f).resolve()
            if p.exists() and p not in linked:
                R.err('README.md', f'в оглавлении нет ссылки на {f}')
        for d in sorted((ROOT / 'code').iterdir()) if (ROOT / 'code').is_dir() else []:
            if d.is_dir() and any(x.is_file() for x in d.rglob('*')) and d.resolve() not in linked:
                R.err('README.md', f'нет ссылки на папку {rel(d)}/')
    R.summary(f'{n_rel} относительных ссылок, из них {n_anchor} с якорем; {n_ext} внешних не проверялись')


# ---------------------------------------------------------------- в) спойлеры <details>
def check_details():
    R.start('спойлеры <details>')
    total = 0
    for f in md_files():
        md = Md.get(f)
        stack = []
        lines = md.lines
        for i, line in md.text_lines():
            for m in re.finditer(r'<details\b[^>]*>|</details\s*>|<summary\b[^>]*>|</summary\s*>', line, re.I):
                tag = m.group(0).lower()
                if tag.startswith('<details'):
                    total += 1
                    stack.append([i, False])
                elif tag.startswith('</details'):
                    if not stack:
                        R.err(f'{md.rel}:{i + 1}', '</details> без открывающего <details>')
                    else:
                        o = stack.pop()
                        if not o[1]:
                            R.warn(f'{md.rel}:{o[0] + 1}', 'в <details> нет <summary>: заголовок спойлера будет «Details»')
                    if line[:m.start()].strip():
                        R.err(f'{md.rel}:{i + 1}', '</details> должен стоять в отдельной строке')
                    elif i == 0 or lines[i - 1].strip():
                        R.err(f'{md.rel}:{i + 1}', 'перед </details> нужна пустая строка')
                elif tag.startswith('<summary'):
                    if stack:
                        stack[-1][1] = True
                else:                                       # </summary>
                    if line[m.end():].strip():
                        R.err(f'{md.rel}:{i + 1}', 'после </summary> в той же строке текст: перенесите его ниже через пустую строку')
                    elif i + 1 < len(lines) and lines[i + 1].strip():
                        R.err(f'{md.rel}:{i + 1}', 'после строки с </summary> нужна пустая строка')
        for o in stack:
            R.err(f'{md.rel}:{o[0] + 1}', '<details> не закрыт')
        if md.unclosed is not None:
            R.err(f'{md.rel}:{md.unclosed + 1}', 'блок ``` не закрыт до конца файла')
    R.summary(f'{total} спойлеров')


# ---------------------------------------------------------------- г) символы в code/
def is_cyr(ch):
    return '\u0410' <= ch <= '\u044f' or ch in '\u0401\u0451'


def check_code_chars():
    R.start('code/: ASCII и кириллица, без табуляции, LF')
    files = code_files()
    for p in files:
        data = p.read_bytes()
        r = rel(p)
        if b'\r' in data:
            ln = data[:data.index(b'\r')].count(b'\n') + 1
            R.err(f'{r}:{ln}', 'перевод строки CRLF (или CR): в репозитории только LF')
        try:
            text = data.decode('utf-8')
        except UnicodeDecodeError as e:
            R.err(r, f'файл не в UTF-8 (байт {e.start}); в репозитории файлы в UTF-8, cp1251 делает tools/make_zip.py')
            continue
        bad = 0
        for ln, line in enumerate(text.split('\n'), 1):
            for col, ch in enumerate(line, 1):
                if ch == '\t':
                    msg = 'табуляция: замените пробелами'
                elif ch == '\r':
                    continue
                elif ord(ch) < 32 or ord(ch) == 127:
                    msg = f'управляющий символ U+{ord(ch):04X}'
                elif ord(ch) > 127 and not is_cyr(ch):
                    name = unicodedata.name(ch, '?')
                    msg = f'символ «{ch}» (U+{ord(ch):04X} {name}) не из ASCII и не кириллица'
                else:
                    continue
                bad += 1
                if bad <= 5:
                    R.err(f'{r}:{ln}:{col}', msg)
        if bad > 5:
            R.err(r, f'и ещё {bad - 5} таких символов')
        if text and not text.endswith('\n'):
            R.warn(r, 'нет перевода строки в конце файла')
    R.summary(f'{len(files)} файлов .c/.h')


# ---------------------------------------------------------------- д) блок ПОДКЛЮЧЕНИЕ
SOCK = re.compile(r'(?<![A-Za-z0-9_])P([A-D])([0-7])(?:\s*\.\.\s*(?:P([A-D]))?([0-7]))?(?![A-Za-z0-9_])')


def sockets(text):
    res, bad = [], []
    for m in SOCK.finditer(text):
        port, a = m.group(1), int(m.group(2))
        if m.group(4) is None:
            res.append(f'P{port}{a}')
            continue
        if m.group(3) and m.group(3) != port:
            bad.append(f'диапазон через разные порты: {m.group(0)}')
            continue
        b = int(m.group(4))
        if b < a:
            bad.append(f'диапазон наоборот: {m.group(0)}')
            continue
        res += [f'P{port}{k}' for k in range(a, b + 1)]
    return res, bad


def short(socks):
    """PB4, PB5, PB6, PB7 -> PB4..PB7 (для сообщений)."""
    order = sorted(set(socks), key=lambda s: ('ACBD'.index(s[1]), int(s[2])))
    out, i = [], 0
    while i < len(order):
        j = i
        while j + 1 < len(order) and order[j + 1][1] == order[i][1] and int(order[j + 1][2]) == int(order[j][2]) + 1:
            j += 1
        if j - i >= 2:
            out.append(f'{order[i]}..{order[j]}')
        else:
            out.extend(order[i:j + 1])
        i = j + 1
    return ', '.join(out)


def check_connections():
    R.start('ПОДКЛЮЧЕНИЕ в шапках программ')
    progs, fragments = programs()
    ok = 0
    for main, files in progs:
        r = rel(main)
        lines = read(main).split('\n')
        head = []                                   # шапка: комментарии и пустые строки до первой строки кода
        for i, line in enumerate(lines):
            s = line.strip()
            if s and not s.startswith('//'):
                break
            head.append((i, s))
        start = next((i for i, s in head if 'ПОДКЛЮЧЕНИЕ' in s), None)
        if start is None:
            any_place = next((i for i, l in enumerate(lines) if 'ПОДКЛЮЧЕНИЕ' in l), None)
            R.err(f'{r}:{(any_place or 0) + 1}', 'нет блока ПОДКЛЮЧЕНИЕ в шапке (комментарий до первой строки кода)'
                  if any_place is None else 'блок ПОДКЛЮЧЕНИЕ не в шапке файла')
            continue
        used, errors, total_line, n_declared = set(), 0, None, None
        for i, s in head:
            if i < start:
                continue
            body = s[2:].strip() if s.startswith('//') else s
            if i == start:                          # «ПОДКЛЮЧЕНИЕ (модуль периферии -> "Контроллер ПУ"):»
                body = re.sub(r'^.*?ПОДКЛЮЧЕНИЕ\s*(\([^)]*\))?\s*:?', '', body).strip()
            m = re.match(r'^гн[её]зд\s+занято\s*:\s*(\d+)\s+из\s+16\b', body, re.I)
            if m:
                total_line, n_declared = i, int(m.group(1))
                if not body.startswith('гнёзд занято:'):
                    R.err(f'{r}:{i + 1}', 'последняя строка блока пишется ровно «гнёзд занято: N из 16»')
                    errors += 1
                break
            if '->' not in body:
                continue
            if body.count('->') > 1:
                R.err(f'{r}:{i + 1}', 'в строке несколько «->»: одна связь на строку («что -> гнёзда (уровень)»)')
                errors += 1
                continue
            right = body.split('->', 1)[1].split('(', 1)[0]
            if re.search(r'P[A-D][0-7]\s*-\s*P?[A-D]?[0-7]', right):
                R.err(f'{r}:{i + 1}', 'диапазон гнёзд пишите через «..»: PD0..PD7')
                errors += 1
            found, bad = sockets(right)
            for b in bad:
                R.err(f'{r}:{i + 1}', b)
                errors += 1
            if not found:
                if not re.search(r'GND|общ', right, re.I):
                    R.err(f'{r}:{i + 1}', f'в строке с «->» нет гнёзд справа от стрелки: {body}')
                    errors += 1
                continue
            for sname in found:
                if sname not in PANEL:
                    R.err(f'{r}:{i + 1}', f'гнезда {sname} нет на панели (есть только PC0, PC1, PB2..PB7, PD0..PD7)')
                    errors += 1
            used |= set(found)
        if total_line is None:
            R.err(f'{r}:{start + 1}', 'блок ПОДКЛЮЧЕНИЕ не заканчивается строкой «гнёзд занято: N из 16» '
                  '(или между ними есть строка кода)')
            continue
        if n_declared > 16:
            R.err(f'{r}:{total_line + 1}', f'гнёзд занято {n_declared}, а на панели их 16')
            errors += 1
        if n_declared != len(used & PANEL) and not errors:
            R.err(f'{r}:{total_line + 1}', f'написано {n_declared} из 16, а в строках с «->» '
                  f'{len(used & PANEL)} различных гнёзд: {short(used & PANEL) or "ни одного"}')
            errors += 1
        if not errors:
            ok += 1
    R.summary(f'{len(progs)} программ, без ошибок {ok}; фрагментов без main(): {len(fragments)} '
              f'(не проверяются: {", ".join(rel(p) for p in fragments) or "нет"})')


# ---------------------------------------------------------------- е) запрещённые слова и даты
W = r'(?<![А-Яа-яЁёA-Za-z])'                    # граница слова слева
E = r'(?![А-Яа-яЁёA-Za-z])'                     # граница слова справа
MONTHS = 'января|февраля|марта|апреля|мая|июня|июля|августа|сентября|октября|ноября|декабря'
# (что ищем, почему нельзя, где: 'all' = README, docs и code; 'docs' = только README и docs)
FORBIDDEN = [
    (re.compile(W + r'пятни[цч][а-яё]*', re.I), 'день недели: пишите ссылку на главу', 'all'),
    (re.compile(W + r'суббот[а-яё]*', re.I), 'день недели: пишите ссылку на главу', 'all'),
    (re.compile(W + r'(понедельник|вторник|четверг|воскресень)[а-яё]*', re.I), 'день недели', 'all'),
    (re.compile(r'(?<![A-Za-z])users[\\/](?!<|%username%|имя|user[\\/]|public)', re.I),
     'путь с именем пользователя (пример пишите как C:\\Users\\<имя>)', 'all'),
    (re.compile(r'(?<![0-9])[0-3]?[0-9]\s+(' + MONTHS + r')' + E, re.I), 'дата', 'all'),
    (re.compile(r'(?<![0-9.])(0?[1-9]|[12][0-9]|3[01])\.(0?[1-9]|1[0-2])\.(19|20)[0-9]{2}(?![0-9.])'), 'дата', 'all'),
    (re.compile(W + r'пар(а|е|у|ы|ой|ах)\s*№?\s*[1-9](?![0-9])', re.I), 'пишите «занятие N»', 'docs'),
    (re.compile(W + r'на\s+пар(е|ах)' + E, re.I), 'пишите «на занятии»', 'docs'),
]
# «пары» как занятия нельзя, «пары проводов» (чего-то во мн. ч.) можно
PARY = re.compile(W + r'пары' + E + r'(?:[ \t]+([А-Яа-яЁё]+))?', re.I)
PARY_OK = re.compile(r'(ов|ев|ей|ий|ок|ек|ёк)$|^(гнёзд|гнезд|цифр|строк|скобок|кнопок|точек|клавиш)$', re.I)
# «выходной» в электронике (выходной сигнал, ток, уровень ...) разрешён, «выходные» как дни - нет
OUT_WORD = re.compile(W + r'выходн[а-яё]*', re.I)
OUT_OK = ('сигнал', 'ток', 'уров', 'вывод', 'лини', 'каскад', 'напряж', 'порт', 'регистр', 'частот', 'шим',
          'сопротивл', 'мощн', 'состоян', 'буфер', 'контакт', 'гнёзд', 'гнезд', 'значен', 'импульс', 'цеп',
          'транзистор', 'ключ', 'резистор', 'данн', 'параметр', 'характерист', 'нагрузк', 'ножк', 'бит', 'пин',
          'режим', 'разъ', 'клемм', 'меандр', 'фильтр', 'звук', 'код', 'устройств', 'элемент', 'величин',
          'канал', 'функци', 'сравн', 'триггер', 'формир', 'интерфейс', 'ступен', 'провод', 'шин', 'ячейк')


def check_words():
    R.start('запрещённые слова и даты')
    docs, code = md_files(), code_files()
    for f in docs + code:
        r = rel(f)
        is_doc = f.suffix == '.md'
        for ln, line in enumerate(read(f).split('\n'), 1):
            for rx, why, scope in FORBIDDEN:
                if scope == 'docs' and not is_doc:
                    continue
                for m in rx.finditer(line):
                    R.err(f'{r}:{ln}', f'«{m.group(0).strip()}»: {why}')
            if not is_doc:
                continue
            for m in PARY.finditer(line):
                if m.group(1) and PARY_OK.search(m.group(1)):
                    continue
                R.err(f'{r}:{ln}', '«пары»: если это занятия, пишите «занятия»; если «пара чего-то», '
                      'перефразируйте (например, «два соседних»)')
            for m in OUT_WORD.finditer(line):
                nxt = re.match(r'[\s«"(]*([А-Яа-яЁёA-Za-z]+)', line[m.end():])
                if nxt and nxt.group(1).lower().startswith(OUT_OK):
                    continue
                R.err(f'{r}:{ln}', f'«{m.group(0)}»: если речь о выходных днях, уберите; если о выходе МК, '
                      'допишите, что именно (выходной сигнал, уровень ...)')
    R.summary(f'README и docs: {len(docs)} файлов (полный список); code: {len(code)} файлов '
              '(имена, пути пользователя, даты, дни недели)')


# ---------------------------------------------------------------- ж) заголовки глав и навигация
def nav_line(k):
    files = [c[0].split('/')[-1] for c in CHAPTERS]
    titles = [c[1] for c in CHAPTERS]
    parts = []
    if k > 0:
        parts.append(f'[← {titles[k - 1]}]({files[k - 1]})')
    parts.append('[Оглавление](../README.md)')
    if k < len(CHAPTERS) - 1:
        parts.append(f'[{titles[k + 1]} →]({files[k + 1]})')
    return ' · '.join(parts)


def check_chapters():
    R.start('заголовки глав и навигация')
    present = 0
    for k, (f, title) in enumerate(CHAPTERS):
        p = ROOT / f
        if not p.exists():
            R.err(f, 'нет файла главы')
            continue
        present += 1
        md = Md.get(p)
        hs = md.headings()
        h1 = [h for h in hs if h[1] == 1]
        if not hs or hs[0][1] != 1:
            R.err(f'{f}:{hs[0][0] + 1 if hs else 1}', f'глава должна начинаться с «# {title}»')
        elif hs[0][3] != title or not hs[0][2].startswith('# '):
            R.err(f'{f}:{hs[0][0] + 1}', f'заголовок «{hs[0][2]}», а по брифу «# {title}»')
        if len(h1) > 1:
            R.err(f'{f}:{h1[1][0] + 1}', f'второй заголовок первого уровня: «{h1[1][2]}» (H1 в главе один)')
        texts = {h[2].strip() for h in hs}
        for need in REQUIRED.get(f, []):
            if need not in texts:
                R.err(f, f'нет обязательного заголовка «{need}» (на него ссылаются другие главы)')
        last = next((l.strip() for l in reversed(md.lines) if l.strip()), '')
        want = nav_line(k)
        if f.endswith('spravka.md'):
            if '[Оглавление](../README.md)' not in last:
                R.err(f, f'последняя строка не навигация; ожидается: {want}')
            elif last != want:
                R.warn(f, f'навигация «{last}»; предлагается: {want}')
        elif last != want:
            R.err(f, f'последняя строка должна быть навигацией:\n        {want}\n      сейчас:\n        {last}')
        if not f.endswith('spravka.md') and not any(SELFCHECK in h[3] for h in hs):
            R.warn(f, f'нет раздела «{SELFCHECK}»')
    extra = sorted(set(rel(p) for p in (ROOT / 'docs').glob('*.md')) - {c[0] for c in CHAPTERS}) \
        if (ROOT / 'docs').is_dir() else []
    for e in extra:
        R.warn(e, 'файла нет в таблице глав брифа')
    R.summary(f'{present} из {len(CHAPTERS)} файлов глав на месте')


# ---------------------------------------------------------------- з) длинное тире и диапазоны
def check_dashes():
    R.start('длинное тире «—» (только подсчёт)')
    total = 0
    for f in md_files():
        md = Md.get(f)
        n = 0
        para, para_start, heavy = 0, 0, []
        for i, line in enumerate(md.lines + ['']):
            if i < len(md.lines) and md.code[i]:
                continue
            if not line.strip():
                if para > 1:
                    heavy.append(f'{para_start + 1} ({para} шт.)')
                para = 0
                continue
            if para == 0:
                para_start = i
            c = line.count('—')
            n += c
            para += c
        for i, line in md.text_lines():
            for m in re.finditer(r'(?<![A-Za-z0-9])P[A-D][0-7]-P[A-D][0-7](?![0-9])', line):
                R.warn(f'{md.rel}:{i + 1}', f'диапазон «{m.group(0)}»: в тексте пишется с коротким тире «–» без пробелов')
        total += n
        if n:
            extra = f'; абзацы с двумя и больше, строка: {", ".join(heavy)}' if heavy else ''
            R.warn(md.rel, f'«—» {n} шт.{extra}')
    R.summary(f'всего «—»: {total} (по брифу по умолчанию ни одного на абзац)')


# ---------------------------------------------------------------- и) таблицы
DELIM = re.compile(r'^\s*\|?\s*:?-+:?\s*(\|\s*:?-+:?\s*)*\|?\s*$')


def cells(row):
    s = row.strip()
    s = s[1:] if s.startswith('|') else s
    s = s[:-1] if s.endswith('|') and not s.endswith('\\|') else s
    return re.split(r'(?<!\\)\|', s)


def pipe_in_code(row):
    """Первый `код` в строке таблицы, где «|» не экранирован (GitHub режет по нему ячейку)."""
    for m in CODESPAN.finditer(row):
        if re.search(r'(?<!\\)\|', m.group(2)):
            return m.group(0)
    return None


def check_tables():
    R.start('таблицы')
    n = 0
    for f in md_files():
        md = Md.get(f)
        lines = md.lines
        rows = {}                                   # номер строки -> число ячеек в заголовке (0 = не таблица)
        i = 0
        while i < len(lines) - 1:
            if md.code[i] or md.comment[i] or '|' not in lines[i] or md.code[i + 1] or not DELIM.match(lines[i + 1]):
                i += 1
                continue
            n += 1
            head = len(cells(lines[i]))
            rows[i] = head
            if len(cells(lines[i + 1])) != head:
                R.err(f'{md.rel}:{i + 2}', f'в строке-разделителе {len(cells(lines[i + 1]))} ячеек, в заголовке {head}: '
                      'GitHub не распознает таблицу')
            k = i + 2
            while k < len(lines) and lines[k].strip() and '|' in lines[k] and not md.code[k]:
                rows[k] = head
                k += 1
            i = k
        for k, line in enumerate(lines):            # строки, похожие на таблицу, но без строки-разделителя
            if k not in rows and not md.code[k] and not md.comment[k] and line.lstrip().startswith('|'):
                rows[k] = 0
        for k in sorted(rows):
            bad = pipe_in_code(lines[k])
            if bad:
                R.err(f'{md.rel}:{k + 1}', f'«|» внутри кода {bad} в строке таблицы: GitHub разрежет по нему ячейку, '
                      'остаток пропадёт; пишите «\\|»')
            elif rows[k] and len(cells(lines[k])) > rows[k]:
                R.err(f'{md.rel}:{k + 1}', f'в строке {len(cells(lines[k]))} ячеек, в заголовке {rows[k]}: '
                      'лишние GitHub выбросит')
    R.summary(f'{n} таблиц')


# ---------------------------------------------------------------- к) у каждой программы есть тест логики
def check_tested():
    R.start('тесты логики на каждую программу (только предупреждения)')
    inc = set()
    for tf in (ROOT / 'tests' / 'logic').rglob('*.c') if (ROOT / 'tests' / 'logic').is_dir() else []:
        for m in re.finditer(r'#include\s+"([^"]+)"', read(tf)):
            inc.add(m.group(1))
    progs, _ = programs()
    miss = []
    for main, files in progs:
        names = {f.relative_to(ROOT / 'code').as_posix() for f in files}
        if not names & inc:
            miss.append(rel(main))
            R.warn(rel(main), 'ни один тест в tests/logic/ не подключает эту программу')
    R.summary(f'{len(progs) - len(miss)} из {len(progs)} программ подключены тестами')


# ---------------------------------------------------------------- запуск
def main():
    checks = [check_fragments, check_links, check_details, check_code_chars, check_connections,
              check_words, check_chapters, check_dashes, check_tables, check_tested]
    for c in checks:
        try:
            c()
        except Exception as e:                  # сбой самой проверки = ошибка, а не молчание
            R.err('tests/check_docs.py', f'проверка «{R.cur[0]}» упала: {type(e).__name__}: {e}')
    n_md, n_code = len(md_files()), len(code_files())
    print(f'check_docs: {n_md} файлов .md (README + docs), {n_code} файлов в code/')
    errors = warnings = 0
    for title, summ, errs, warns in R.sections:
        errors += len(errs)
        warnings += len(warns)
        if errs:
            status = f'ОШИБКИ: {len(errs)}'
        elif warns:
            status = f'ok, предупреждений: {len(warns)}'
        else:
            status = 'ok'
        print(f'  [{status}] {title}: {summ}')
        if not QUIET:
            for e in errs[:60]:
                print(f'      {e}')
            if len(errs) > 60:
                print(f'      ... и ещё {len(errs) - 60}')
            for w in warns[:30]:
                print(f'      (предупреждение) {w}')
            if len(warns) > 30:
                print(f'      ... и ещё {len(warns) - 30} предупреждений')
    print(f'check_docs: ошибок {errors}, предупреждений {warnings}')
    return 1 if errors else 0


if __name__ == '__main__':
    sys.exit(main())
