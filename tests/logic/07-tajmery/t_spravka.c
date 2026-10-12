/* Справка (docs/spravka.md): числа в таблицах пересчитываются заново и сверяются.
   - «Коды сегментов»: код каждого символа считается по списку горящих сегментов (a = бит 0 ... g = бит 6,
     dp = бит 7) и сравнивается с колонками «dp g f e d c b a» и hex; массив kod7 совпадает с таблицей.
   - «Таймерные числа при 8 МГц»: тик и переполнения; для прерываний и меандров каждая пара N, OCR даёт
     заявленную частоту, «ровно» перечисляет все точные варианты, «≈» - лучшее приближение, «нельзя» - честно.
   - частоты ШИМ.
   Программы здесь нет; тест запускается из корня репозитория (так делает tests/logic.sh). */
#include "sim.h"
int prog_main(void){ return 0; }          /* sim.h ссылается на prog_main, программа тесту не нужна */

#define FCPU 8000000.0
static const int DIV01[] = {1, 8, 64, 256, 1024, 0};
static const int DIV2[]  = {1, 8, 32, 64, 128, 256, 1024, 0};

static char L[1500][700];
static int nl = 0;

static double fabs_(double x){ return x < 0 ? -x : x; }   /* без libm: logic.sh собирает без -lm */
static long floor_(double x){ long i = (long)x; return (x < i) ? i - 1 : i; }
static long lround_(double x){ return floor_(x + 0.5); }
static int has_div(const int *d, int n){ for(; *d; d++) if(*d == n) return 1; return 0; }
static int max_div(const int *d){ int m = 0; for(; *d; d++) if(*d > m) m = *d; return m; }

/* число по-русски ("2,048", "31250") с позиции s; *dec = цифр после запятой; *end = конец числа */
static double rnum(const char *s, int *dec, const char **end){
  char b[64]; int n = 0, d = -1;
  while(*s == ' ') s++;
  while(n < 60 && ((*s >= '0' && *s <= '9') || *s == ',')){
    if(*s == ',') d = 0; else if(d >= 0) d++;
    b[n++] = (*s == ',') ? '.' : *s; s++;
  }
  b[n] = 0;
  if(dec) *dec = d < 0 ? 0 : d;
  if(end) *end = s;
  return n ? atof(b) : -1;
}
/* совпадает ли округлённое значение из таблицы с настоящим */
static int near(double shown, int dec, double real){
  double tol = 0.5; for(int i = 0; i < dec; i++) tol /= 10;
  return fabs_(shown - real) <= tol * 1.0001 + 1e-12;
}
/* время "8 мкс", "2,048 мс", "8,389 с" -> секунды */
static int time_ok(const char *s, double real){
  int dec; const char *e; double v = rnum(s, &dec, &e);
  while(*e == ' ') e++;
  if(!strncmp(e, "мкс", strlen("мкс"))) return near(v, dec, real * 1e6);
  if(!strncmp(e, "мс", strlen("мс")))  return near(v, dec, real * 1e3);
  if(!strncmp(e, "с", strlen("с")))   return near(v, dec, real);
  return 0;
}
/* частота в скобках: "(488,28 Гц)" */
static int freq_in_paren_ok(const char *s, double real){
  const char *p = strchr(s, '(');
  if(!p) return 0;
  int dec; double v = rnum(p + 1, &dec, 0);
  return near(v, dec, real);
}
/* ячейки строки таблицы: | a | b | c | ... ; возвращает число ячеек */
static int cells(const char *line, char c[8][400]){
  int n = 0; const char *p = strchr(line, '|');
  while(p && n < 8){
    const char *q = strchr(p + 1, '|');
    if(!q) break;
    int len = (int)(q - p - 1); if(len > 399) len = 399;
    memcpy(c[n], p + 1, len); c[n][len] = 0;
    char *t = c[n]; while(*t == ' ') t++;                          /* обрезать пробелы */
    memmove(c[n], t, strlen(t) + 1);
    for(int k = (int)strlen(c[n]) - 1; k >= 0 && c[n][k] == ' '; k--) c[n][k] = 0;
    n++; p = q;
  }
  return n;
}
static int seg_code(const char *s){       /* "a b c d e f" -> 0x3F; "ничего" -> 0 */
  int c = 0;
  if(!strcmp(s, "ничего")) return 0;
  for(const char *p = s; *p; ){
    while(*p == ' ') p++;
    if(!*p) break;
    if(p[0] == 'd' && p[1] == 'p'){ c |= 0x80; p += 2; }
    else if(*p >= 'a' && *p <= 'g' && (p[1] == ' ' || p[1] == 0)){ c |= 1 << (*p - 'a'); p++; }
    else return -1;
  }
  return c;
}

static int seg_rows = 0, seg_bad = 0, kod7_ok = 0, digits[10];
static int ovf_rows = 0, ovf_bad = 0, frq_rows = 0, frq_bad = 0, pwm_rows = 0, pwm_bad = 0;

static void check_seg(char c[8][400]){
  int code = seg_code(c[1]);
  if(code < 0){ printf("  непонятные сегменты: %s\n", c[1]); seg_bad++; return; }
  char bits[16]; snprintf(bits, sizeof bits, "`%d%d%d%d %d%d%d%d`", (code >> 7) & 1, (code >> 6) & 1, (code >> 5) & 1,
                          (code >> 4) & 1, (code >> 3) & 1, (code >> 2) & 1, (code >> 1) & 1, code & 1);
  char hex[16]; snprintf(hex, sizeof hex, "`0x%02X`", code);
  if(strcmp(bits, c[2]) || strcmp(hex, c[3])){ printf("  символ %s: по сегментам %s %s, в таблице %s %s\n", c[0], bits, hex, c[2], c[3]); seg_bad++; }
  if(strlen(c[0]) == 1 && c[0][0] >= '0' && c[0][0] <= '9') digits[c[0][0] - '0'] = code;
  seg_rows++;
}
static void check_ovf(char c[8][400]){
  int N = atoi(c[0]);
  double tick = N / FCPU;
  int ok = time_ok(c[2], tick) && time_ok(c[3], tick * 256) && freq_in_paren_ok(c[3], 1 / (tick * 256));
  if(has_div(DIV01, N)) ok = ok && time_ok(c[4], tick * 65536) && freq_in_paren_ok(c[4], 1 / (tick * 65536))
                          && !strcmp(c[1], "все три");
  else ok = ok && has_div(DIV2, N) && strstr(c[4], "нет") && !strcmp(c[1], "только Timer2");
  if(!ok){ printf("  тик/переполнение: строка N=%d не сходится\n", N); ovf_bad++; }
  ovf_rows++;
}
/* одна ячейка таблицы частот: таймер bits, делители divs; k = 1 прерывания, 2 меандр */
static int check_cell(const char *cell, double f, int k, int bits, const int *divs){
  int mx = (1 << bits) - 1;
  int nex = 0, exN[8], exO[8]; int bN = 0, bO = -1; double berr = 1e9;
  for(const int *d = divs; *d; d++){
    double x = FCPU / (k * (*d) * f);            /* = OCR + 1 */
    long xi = lround_(x);
    if(fabs_(x - xi) < 1e-9 && xi >= 1 && xi <= mx + 1){ exN[nex] = *d; exO[nex] = (int)xi - 1; nex++; }
    for(long o = floor_(x) - 1; o <= floor_(x); o++){
      if(o < 0 || o > mx) continue;
      double fa = FCPU / (k * (*d) * (o + 1)), err = fabs_(fa - f) / f;
      if(err < berr - 1e-15){ berr = err; bN = *d; bO = (int)o; }
    }
  }
  if(!strncmp(cell, "ровно:", strlen("ровно:"))){
    int n = 0; const char *p = cell;
    while((p = strstr(p, "N=")) != 0){
      int N = atoi(p + 2); const char *q = strstr(p, "OCR="); if(!q) return 0;
      int O = atoi(q + 4);
      if(n >= nex || exN[n] != N || exO[n] != O) return 0;
      n++; p = q;
    }
    return n == nex && n > 0;
  }
  if(!strncmp(cell, "≈", strlen("≈"))){
    if(nex) return 0;
    const char *p = strstr(cell, "N="), *q = strstr(cell, "OCR=");
    if(!p || !q) return 0;
    int N = atoi(p + 2), O = atoi(q + 4);
    if(N != bN || O != bO) return 0;
    const char *r = strchr(q, ':'); if(!r) return 0;
    int dec; const char *e; double v = rnum(r + 1, &dec, &e);
    double fa = FCPU / (k * N * (O + 1.0));
    if(!near(v, dec, fa)) return 0;
    const char *pc = strchr(e, '('); if(!pc) return 0;
    double perc = rnum(pc + 1, &dec, 0);
    return near(perc, dec, fabs_(fa - f) / f * 100);
  }
  if(!strncmp(cell, "нельзя", strlen("нельзя"))){
    if(nex || bO >= 0) return 0;
    const char *p = strstr(cell, "меньше"); if(!p) return 0;
    int dec; double v = rnum(p + strlen("меньше"), &dec, 0);
    return near(v, dec, FCPU / (k * max_div(divs) * (mx + 1.0)));
  }
  return 0;
}
static void check_freq(char c[8][400], int k){
  double f = atof(c[0]);
  int ok = check_cell(c[1], f, k, 16, DIV01) && check_cell(c[2], f, k, 8, DIV01) && check_cell(c[3], f, k, 8, DIV2);
  if(!ok){ printf("  %s %s: числа не сходятся с расчётом\n", k == 1 ? "прерывания" : "меандр", c[0]); frq_bad++; }
  frq_rows++;
}
static void check_pwm(char c[8][400]){
  int N = atoi(c[0]), d1, d2;
  double v1 = rnum(c[1], &d1, 0), v2 = rnum(c[2], &d2, 0);
  int ok = near(v1, d1, FCPU / (N * 256.0)) && near(v2, d2, FCPU / (N * 510.0))
           && (has_div(DIV01, N) ? !strstr(c[0], "Timer2") : (has_div(DIV2, N) && strstr(c[0], "только Timer2")));
  if(!ok){ printf("  ШИМ: строка N=%d не сходится\n", N); pwm_bad++; }
  pwm_rows++;
}

int main(void){
  FILE *fp = fopen("docs/spravka.md", "r");
  CHECK(fp != 0, "docs/spravka.md открывается (тест запускается из корня репозитория)");
  if(!fp) return sim_done("справка: таблицы");
  while(nl < 1500 && fgets(L[nl], sizeof L[nl], fp)){ L[nl][strcspn(L[nl], "\r\n")] = 0; nl++; }
  fclose(fp);
  for(int i = 0; i < 10; i++) digits[i] = -1;
  enum { NONE, SEG, OVF, IRQ, MEA, PWM } mode = NONE;
  for(int i = 0; i < nl; i++){
    const char *s = L[i];
    if(!strncmp(s, "## ", 3) || !strncmp(s, "### ", 4)){
      mode = strstr(s, "Коды сегментов") ? SEG : strstr(s, "Тик и переполнение") ? OVF :
             strstr(s, "Прерывания по совпадению") ? IRQ : strstr(s, "Меандр на выводе") ? MEA :
             strstr(s, "Частоты ШИМ") ? PWM : NONE;
      continue;
    }
    if(mode == SEG && !strncmp(s, "char kod7[10]={", 15)){
      int v[10], n = 0; const char *p = s + 15;
      while(n < 10 && (p = strstr(p, "0x")) != 0){ v[n++] = (int)strtol(p, 0, 16); p += 2; }
      kod7_ok = (n == 10);
      for(int d = 0; d < 10 && kod7_ok; d++) if(v[d] != digits[d]) kod7_ok = 0;
      continue;
    }
    if(s[0] != '|' || strstr(s, "|---")) continue;
    char c[8][400]; int n = cells(s, c);
    if(mode == SEG && n == 4 && strcmp(c[0], "Символ")) check_seg(c);
    else if(mode == OVF && n == 5 && strcmp(c[0], "Делитель N")) check_ovf(c);
    else if((mode == IRQ || mode == MEA) && n == 4 && strcmp(c[0], "Частота")) check_freq(c, mode == IRQ ? 1 : 2);
    else if(mode == PWM && n == 3 && strcmp(c[0], "N")) check_pwm(c);
  }
  CHECK(seg_rows >= 30 && seg_bad == 0, "коды сегментов: каждый hex и разбор по битам совпадают с горящими сегментами");
  CHECK(digits[0] == 0x3F && digits[1] == 0x06 && digits[2] == 0x5B && digits[3] == 0x4F,
        "коды 0..3 как проверено на стенде: 0x3F 0x06 0x5B 0x4F");
  CHECK(kod7_ok, "массив kod7 в справке совпадает с таблицей цифр");
  CHECK(ovf_rows == 7 && ovf_bad == 0, "тик и переполнение 8/16 бит для всех 7 делителей");
  CHECK(frq_rows == 14 && frq_bad == 0, "прерывания и меандры 1..1000 Гц: все N и OCR дают заявленные частоты");
  CHECK(pwm_rows == 7 && pwm_bad == 0, "частоты ШИМ Fast PWM и Phase Correct");
  return sim_done("справка: таблицы");
}
