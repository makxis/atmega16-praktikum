/* tests/sim.h - модель ATmega16 и внешнего мира для проверки логики программ курса на ПК.

   Тест - один файл tests/logic/<глава>/<имя>.c:
       #include "sim.h"
       #include "05-indikator/444.c"        // копия программы из tests/.work (main -> prog_main)
       int main(void){ sim_start(); CHECK(disp_digit(0)==3, "..."); return sim_done("05/444"); }
   Сборка: gcc -std=gnu99 -w -I tests/stub -I tests -I tests/.work  (см. tests/logic.sh).

   Что моделируется:
   - выводы портов A..D: выход (DDR=1) даёт бит PORT (или сигнал таймера на OC0/OC1A/OC1B/OC2, если включены
     COM-биты); вход читает то, что к нему подключено проводами (pin_wire, клавиатура), иначе внешний источник
     (pin_ext, кнопка), иначе подтяжку (PORT=1 и PUD=0 в SFIOR) = 1, иначе вход "висит" и читается 0;
   - прерывания: бит I в SREG (sei/cli), разрешения TIMSK и GICR, флаги; флаги TIFR/GIFR сбрасываются записью 1;
     у разрешённого прерывания обязан быть обработчик (иначе на МК - перезапуск, здесь - FAIL);
   - внешние прерывания INT0 (PD2), INT1 (PD3), INT2 (PB2) по битам ISC; смена ISC2 сама ставит флаг INTF2;
   - таймеры 0, 1, 2 в модельном времени (run_ms, _delay_ms): делитель CS, режимы WGM (Normal, CTC, Fast PWM,
     Phase Correct), OCR, TOP, флаги и обработчики OVF/COMP, выходы OC (переключение/сброс/установка, ШИМ).
   Не моделируется: чтение флагов TIFR/GIFR программой (всегда 0), двойная буферизация OCR в ШИМ-режимах
   (новое OCR действует сразу, поэтому перед замером pin_duty() дайте run_ms(5)), захват ICP1, внешний такт
   T0/T1, асинхронный Timer2, АЦП, USART, SPI, TWI, длительность самих обработчиков (они мгновенные),
   промежуточные состояния портов внутри одного обработчика (модель видит порты после его возврата).
   Дребезг кнопок и клавиш строго периодический: 6 отскоков, по одному на вызов T0.
*/
#ifndef SIM_H
#define SIM_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <setjmp.h>
#include <avr/io.h>

#ifndef SIM_F_CPU
#define SIM_F_CPU 8000000.0
#endif

/* ---------------- регистры */
volatile uint8_t PORTA, PORTB, PORTC, PORTD, DDRA, DDRB, DDRC, DDRD;
volatile uint8_t SREG, SPL, SPH, SFIOR, MCUCR, MCUCSR, GICR, GIFR, TIMSK, TIFR;
volatile uint8_t TCCR0, TCNT0, OCR0, TCCR1A, TCCR1B, TCCR2, TCNT2, OCR2, ASSR;
volatile uint16_t TCNT1, OCR1A, OCR1B, ICR1;
volatile uint8_t ACSR, ADMUX, ADCSRA, ADCL, ADCH, WDTCR, OSCCAL, OCDR;
volatile uint8_t UDR, UCSRA, UCSRB, UCSRC, UBRRL, UBRRH, SPCR, SPSR, SPDR;
volatile uint8_t TWBR, TWSR, TWAR, TWDR, TWCR, EEARL, EEARH, EEDR, EECR, SPMCR;
volatile uint16_t ADC, ADCW, EEAR;
uint8_t sim_eeprom[512];

/* ---------------- обработчики прерываний: слабые ссылки (нет в программе = NULL) */
#define SIM_VECTORS(X) X(INT0) X(INT1) X(TIMER2_COMP) X(TIMER2_OVF) X(TIMER1_CAPT) X(TIMER1_COMPA) \
  X(TIMER1_COMPB) X(TIMER1_OVF) X(TIMER0_OVF) X(SPI_STC) X(USART_RXC) X(USART_UDRE) X(USART_TXC) X(ADC) \
  X(EE_RDY) X(ANA_COMP) X(TWI) X(INT2) X(TIMER0_COMP) X(SPM_RDY)
#define SIM_WEAK(n) void n##_vect(void) __attribute__((weak));
SIM_VECTORS(SIM_WEAK)
int prog_main(void);

enum { V_INT0, V_INT1, V_TIMER2_COMP, V_TIMER2_OVF, V_TIMER1_CAPT, V_TIMER1_COMPA, V_TIMER1_COMPB,
       V_TIMER1_OVF, V_TIMER0_OVF, V_INT2, V_TIMER0_COMP, V_N };
static const char *sim_vname[V_N] = {"INT0", "INT1", "TIMER2_COMP", "TIMER2_OVF", "TIMER1_CAPT", "TIMER1_COMPA",
  "TIMER1_COMPB", "TIMER1_OVF", "TIMER0_OVF", "INT2", "TIMER0_COMP"};
static void (*sim_vfun(int v))(void){
  switch(v){
    case V_INT0: return INT0_vect;            case V_INT1: return INT1_vect;
    case V_TIMER2_COMP: return TIMER2_COMP_vect; case V_TIMER2_OVF: return TIMER2_OVF_vect;
    case V_TIMER1_CAPT: return TIMER1_CAPT_vect; case V_TIMER1_COMPA: return TIMER1_COMPA_vect;
    case V_TIMER1_COMPB: return TIMER1_COMPB_vect; case V_TIMER1_OVF: return TIMER1_OVF_vect;
    case V_TIMER0_OVF: return TIMER0_OVF_vect; case V_INT2: return INT2_vect;
    case V_TIMER0_COMP: return TIMER0_COMP_vect;
  }
  return 0;
}
/* где лежат бит разрешения и флаг каждого вектора: 0 = TIMSK/TIFR, 1 = GICR/GIFR */
static const int sim_vreg[V_N] = {1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0};
static const int sim_vbit[V_N] = {6, 7, 7, 6, 5, 4, 3, 2, 0, 5, 1};

/* ---------------- проверки */
static int sim_fails = 0;
#define CHECK(c, msg) do{ if(!(c)){ printf("  FAIL: %s  (%s:%d)\n", (msg), __FILE__, __LINE__); sim_fails++; } }while(0)

/* ---------------- состояние модели */
static uint64_t sim_cyc = 0;               /* модельное время в тактах МК */
static long     sim_ncalls[V_N];           /* сколько раз вызван каждый обработчик */
static uint8_t  sim_tifr = 0, sim_gifr = 0;/* настоящие флаги (TIFR/GIFR - регистры "записью 1 сбросить") */
static int      sim_resets = 0;            /* сработало разрешённое прерывание без обработчика */
static int      sim_in_isr = 0, sim_in_after = 0;
static int      sim_ext_lvl[32] = {[0 ... 31] = -1};   /* внешний источник на выводе: -1 нет, 0, 1 */
#define SIM_MAXW 64
static int      sim_wa[SIM_MAXW], sim_wb[SIM_MAXW], sim_nw = 0;  /* провода */
static int      sim_conflicts = 0;         /* два выхода с разными уровнями на одном проводе */
static int      sim_oc_state[4] = {0, 0, 0, 0};   /* OC0, OC1A, OC1B, OC2 */
static const int sim_oc_pin[4] = {1*8+3, 3*8+5, 3*8+4, 3*8+7};  /* PB3, PD5, PD4, PD7 */
static int      sim_int_prev[3] = {-1, -1, -1};  /* прошлые уровни PD2, PD3, PB2 */
static int      sim_prev_set = 0;          /* 0: точку отсчёта уровней снимем при первом sim_after() после настройки */
static int      sim_isc2_prev = 0;         /* после сброса ISC2 = 0 */

#define PIN(port, bit) ((((port) - 'A') << 3) | (bit))
static volatile uint8_t *sim_portr(int p){ static volatile uint8_t *r[4] = {&PORTA, &PORTB, &PORTC, &PORTD}; return r[p]; }
static volatile uint8_t *sim_ddrr(int p){ static volatile uint8_t *r[4] = {&DDRA, &DDRB, &DDRC, &DDRD}; return r[p]; }
static const char *sim_pname(int pin){ static char b[8][6]; static int k = 0; k = (k + 1) & 7;
  snprintf(b[k], 6, "P%c%d", 'A' + (pin >> 3), pin & 7); return b[k]; }

/* COM-биты включены для выхода OC (i = 0 OC0, 1 OC1A, 2 OC1B, 3 OC2)? */
static int sim_tmode0(uint8_t tccr){ return ((tccr >> 6) & 1) | (((tccr >> 3) & 1) << 1); } /* WGMx1:WGMx0 */
static int sim_t1mode(void){ return (TCCR1A & 3) | (((TCCR1B >> 3) & 3) << 2); }
static int sim_oc_on(int i){
  int com;
  switch(i){
    case 0: com = (TCCR0 >> 4) & 3; if(sim_tmode0(TCCR0) & 1 && com == 1) com = 0; return com != 0;
    case 3: com = (TCCR2 >> 4) & 3; if(sim_tmode0(TCCR2) & 1 && com == 1) com = 0; return com != 0;
    case 1: com = (TCCR1A >> 6) & 3; break;
    default: com = (TCCR1A >> 4) & 3; break;
  }
  if(com == 1){ int m = sim_t1mode();      /* в ШИМ-режимах 01 работает только для OC1A в режимах 9, 11, 14, 15 */
    if(m == 0 || m == 4 || m == 12) return 1;
    return i == 1 && (m == 9 || m == 11 || m == 14 || m == 15); }
  return com != 0;
}

/* собственный уровень вывода как источника: 0/1 - выход, -1 - вход (не источник) */
static int sim_drive(int pin){
  int p = pin >> 3, b = pin & 7;
  if(!((*sim_ddrr(p) >> b) & 1)) return -1;
  for(int i = 0; i < 4; i++) if(sim_oc_pin[i] == pin && sim_oc_on(i)) return sim_oc_state[i];
  return (*sim_portr(p) >> b) & 1;
}
int pin_pullup(int pin){
  int p = pin >> 3, b = pin & 7;
  if((*sim_ddrr(p) >> b) & 1) return 0;
  if(SFIOR & (1 << 2)) return 0;           /* PUD = 1: все подтяжки выключены */
  return (*sim_portr(p) >> b) & 1;
}
int pin_out(int pin){ return sim_drive(pin); }

/* уровень на выводе с учётом проводов (сеть соединённых выводов) */
int pin_level(int pin){
  int net[32], n = 0, seen[32] = {0};
  net[n++] = pin; seen[pin] = 1;
  for(int k = 0; k < n; k++)
    for(int w = 0; w < sim_nw; w++){
      int o = -1;
      if(sim_wa[w] == net[k]) o = sim_wb[w]; else if(sim_wb[w] == net[k]) o = sim_wa[w];
      if(o >= 0 && !seen[o]){ seen[o] = 1; net[n++] = o; }
    }
  int drv = -1, ext = -1, pu = 0;
  for(int k = 0; k < n; k++){
    int d = sim_drive(net[k]);
    if(d >= 0){ if(drv >= 0 && d != drv){ sim_conflicts++; drv = 0; } else drv = d; }
    if(sim_ext_lvl[net[k]] >= 0 && ext < 0) ext = sim_ext_lvl[net[k]];
    if(pin_pullup(net[k])) pu = 1;
  }
  if(drv >= 0) return drv;
  if(ext >= 0) return ext;
  return pu ? 1 : 0;                       /* без подтяжки вход "висит": читаем 0 */
}
uint8_t sim_pin(int port){
  uint8_t v = 0;
  for(int b = 0; b < 8; b++) if(pin_level(port * 8 + b)) v |= (uint8_t)(1 << b);
  return v;
}

/* ---------------- прерывания */
static void sim_call(int v){
  void (*f)(void) = sim_vfun(v);
  if(!f){ if(!sim_resets) printf("  FAIL: разрешено прерывание %s, а обработчика нет (МК перезапустится)\n", sim_vname[v]);
          sim_resets++; sim_fails++; return; }
  uint8_t s = SREG; SREG &= (uint8_t)~0x80; sim_in_isr++;
  sim_ncalls[v]++; f();
  sim_in_isr--; SREG = (uint8_t)(s | 0x80);   /* RETI снова разрешает прерывания */
}
static int sim_enabled(int v){ return ((sim_vreg[v] ? GICR : TIMSK) >> sim_vbit[v]) & 1; }
static void sim_flag(int v){ if(sim_vreg[v]) sim_gifr |= (uint8_t)(1 << sim_vbit[v]); else sim_tifr |= (uint8_t)(1 << sim_vbit[v]); }
static int  sim_flagged(int v){ return (((sim_vreg[v] ? sim_gifr : sim_tifr) >> sim_vbit[v]) & 1); }
static void sim_unflag(int v){ if(sim_vreg[v]) sim_gifr &= (uint8_t)~(1 << sim_vbit[v]); else sim_tifr &= (uint8_t)~(1 << sim_vbit[v]); }

static void sim_edges(void){                /* фронты на INT0/INT1/INT2 -> флаги */
  static const int ipin[3] = {3*8+2, 3*8+3, 1*8+2};
  int isc2 = (MCUCSR >> 6) & 1;
  if(isc2 != sim_isc2_prev) sim_gifr |= (1 << 5);  /* смена ISC2 может поставить INTF2 (datasheet): ставим всегда */
  sim_isc2_prev = isc2;
  for(int i = 0; i < 3; i++){
    int l = pin_level(ipin[i]), p = sim_int_prev[i];
    sim_int_prev[i] = l;
    if(i < 2){
      int isc = (MCUCR >> (i * 2)) & 3, hit = 0;
      if(isc == 0) hit = (l == 0);                         /* низкий уровень: пока 0 */
      else if(p >= 0 && p != l) hit = (isc == 1) || (isc == 2 && l == 0) || (isc == 3 && l == 1);
      if(hit){ if(isc == 0){ if(sim_enabled(i) && (SREG & 0x80) && !sim_in_isr) sim_call(i); } else sim_flag(i); }
    } else if(p >= 0 && p != l && ((isc2 == 0 && l == 0) || (isc2 == 1 && l == 1))) sim_flag(V_INT2);
  }
}
/* после любого действия: фронты и смена ISC2 ставят флаги -> записи 1 в GIFR/TIFR их сбрасывают ->
   вызов ожидающих прерываний по приоритету. Модель не видит порядка записей внутри одного куска программы,
   поэтому правильная настройка (сменить ISC2, затем GIFR=(1<<INTF2)) ложного вызова не даёт, а без сброса
   флага даёт, как на МК. Переходы уровней во время начальной настройки фронтами не считаются: точка отсчёта
   снимается при первом sim_after() (после prog_main или на первом _delay_ms в sim_loop). */
static void sim_after(void){
  static const int ipin[3] = {3*8+2, 3*8+3, 1*8+2};
  if(sim_in_after) return;
  sim_in_after = 1;
  if(!sim_prev_set){ for(int i = 0; i < 3; i++) sim_int_prev[i] = pin_level(ipin[i]); sim_prev_set = 1; }
  for(int guard = 0; guard < 1000; guard++){
    sim_edges();
    if(GIFR){ sim_gifr &= (uint8_t)~GIFR; GIFR = 0; }
    if(TIFR){ sim_tifr &= (uint8_t)~TIFR; TIFR = 0; }
    if(!(SREG & 0x80) || sim_in_isr) break;
    int fired = 0;
    for(int v = 0; v < V_N; v++)
      if(sim_flagged(v) && sim_enabled(v)){ sim_unflag(v); sim_call(v); fired = 1; break; }
    if(!fired) break;
  }
  sim_in_after = 0;
}
static void sim_check_vectors(const char *when){
  for(int v = 0; v < V_N; v++)
    if(sim_enabled(v) && !sim_vfun(v)){
      printf("  FAIL: %s: разрешено прерывание %s, а обработчика нет (МК будет перезапускаться)\n", when, sim_vname[v]);
      sim_fails++;
    }
}
long isr_calls(const char *name){
  for(int v = 0; v < V_N; v++) if(!strcmp(sim_vname[v], name)) return sim_ncalls[v];
  printf("  FAIL: isr_calls: нет вектора %s\n", name); sim_fails++; return -1;
}
void isr_calls_reset(void){ memset(sim_ncalls, 0, sizeof sim_ncalls); }

/* ручной вызов обработчика (как в старых тестах): n раз подряд, после каждого - sim_after() */
static void sim_manual(int v, int n){
  if(!sim_vfun(v)){ printf("  FAIL: в программе нет обработчика %s_vect\n", sim_vname[v]); sim_fails++; return; }
  for(int i = 0; i < n; i++){ sim_call(v); sim_after(); }
}
void T0(int n){ sim_manual(V_TIMER0_OVF, n); }
void T0C(int n){ sim_manual(V_TIMER0_COMP, n); }
void T1(int n){ sim_manual(V_TIMER1_COMPA, n); }
void T1B(int n){ sim_manual(V_TIMER1_COMPB, n); }
void T2(int n){ sim_manual(V_TIMER2_OVF, n); }

/* ---------------- таймеры в модельном времени */
static int sim_div01(int cs){ static const int d[8] = {0, 1, 8, 64, 256, 1024, 0, 0}; return d[cs & 7]; } /* 6,7 = вход T0/T1: не выведен */
static int sim_div2(int cs){ static const int d[8] = {0, 1, 8, 32, 64, 128, 256, 1024}; return d[cs & 7]; }
typedef struct { int div; uint64_t next; int up; } sim_tm_t;
static sim_tm_t sim_tm[3] = {{0, 0, 1}, {0, 0, 1}, {0, 0, 1}};
static int sim_tdiv(int t){ return t == 0 ? sim_div01(TCCR0) : t == 1 ? sim_div01(TCCR1B) : sim_div2(TCCR2); }
static void sim_tsync(void){                /* пересчитать, кто из таймеров идёт (регистры могли поменяться) */
  for(int t = 0; t < 3; t++){
    int d = sim_tdiv(t);
    if(d != sim_tm[t].div){ sim_tm[t].div = d; sim_tm[t].next = sim_cyc + (uint64_t)d; }
  }
}
/* действие на выходе OC при совпадении/дне: com - COM-биты, kind 0 не-ШИМ, 1 Fast PWM, 2 Phase Correct */
static void sim_oc_match(int i, int com, int kind, int up){
  if(!sim_oc_on(i)) return;
  if(kind == 0){ if(com == 1) sim_oc_state[i] ^= 1; else if(com == 2) sim_oc_state[i] = 0; else if(com == 3) sim_oc_state[i] = 1; }
  else if(kind == 1){ if(com == 2) sim_oc_state[i] = 0; else if(com == 3) sim_oc_state[i] = 1; else if(com == 1) sim_oc_state[i] ^= 1; }
  else { if(com == 2) sim_oc_state[i] = up ? 0 : 1; else if(com == 3) sim_oc_state[i] = up ? 1 : 0; }
}
static void sim_oc_bottom(int i, int com){ if(!sim_oc_on(i)) return; if(com == 2) sim_oc_state[i] = 1; else if(com == 3) sim_oc_state[i] = 0; }
static void sim_event(int v){ sim_flag(v); }

static void sim_tick8(int t){                /* один такт таймера 0 или 2 */
  volatile uint8_t *cnt = t == 0 ? &TCNT0 : &TCNT2;
  uint8_t ocr = t == 0 ? OCR0 : OCR2, tccr = t == 0 ? TCCR0 : TCCR2;
  int oc = t == 0 ? 0 : 3, com = (tccr >> 4) & 3, m = sim_tmode0(tccr);
  int vovf = t == 0 ? V_TIMER0_OVF : V_TIMER2_OVF, vcmp = t == 0 ? V_TIMER0_COMP : V_TIMER2_COMP;
  uint8_t c = *cnt;
  if(m == 0){                                /* Normal */
    if(c == ocr){ sim_event(vcmp); sim_oc_match(oc, com, 0, 1); }
    *cnt = (uint8_t)(c + 1); if(c == 0xFF) sim_event(vovf);
  } else if(m == 2){                         /* CTC: TOP = OCR */
    if(c == ocr){ sim_event(vcmp); sim_oc_match(oc, com, 0, 1); *cnt = 0; if(c == 0xFF) sim_event(vovf); }
    else { *cnt = (uint8_t)(c + 1); if(c == 0xFF) sim_event(vovf); }
  } else if(m == 3){                         /* Fast PWM: 0..255, вывод меняется в такт после совпадения */
    if(c == 0xFF){ *cnt = 0; sim_event(vovf); if(ocr == 0xFF) sim_event(vcmp); sim_oc_bottom(oc, com); }
    else { if(c == ocr){ sim_event(vcmp); sim_oc_match(oc, com, 1, 1); } *cnt = (uint8_t)(c + 1); }
  } else {                                   /* Phase Correct: 0..255..0 */
    int *up = &sim_tm[t].up;
    if(*up){ if(c == 0xFF){ *up = 0; *cnt = 0xFE; } else *cnt = (uint8_t)(c + 1); }
    else { if(c == 0){ *up = 1; *cnt = 1; } else *cnt = (uint8_t)(c - 1); }
    if(*cnt == ocr){ sim_event(vcmp); sim_oc_match(oc, com, 2, *up); }
    if(*cnt == 0) sim_event(vovf);
  }
}
static void sim_tick16(void){                /* один такт Timer1 */
  int m = sim_t1mode(), kind; uint16_t top;
  switch(m){
    case 0: kind = 0; top = 0xFFFF; break;  case 4: kind = 0; top = OCR1A; break;  case 12: kind = 0; top = ICR1; break;
    case 5: kind = 1; top = 0xFF; break;    case 6: kind = 1; top = 0x1FF; break;  case 7: kind = 1; top = 0x3FF; break;
    case 14: kind = 1; top = ICR1; break;   case 15: kind = 1; top = OCR1A; break;
    case 1: kind = 2; top = 0xFF; break;    case 2: kind = 2; top = 0x1FF; break;  case 3: kind = 2; top = 0x3FF; break;
    case 8: case 10: kind = 2; top = ICR1; break; case 9: case 11: kind = 2; top = OCR1A; break;
    default: kind = 0; top = 0xFFFF; break;
  }
  int ca = (TCCR1A >> 6) & 3, cb = (TCCR1A >> 4) & 3;
  uint16_t c = TCNT1;
  if(kind == 0){
    if(c == OCR1A){ sim_event(V_TIMER1_COMPA); sim_oc_match(1, ca, 0, 1); }
    if(c == OCR1B){ sim_event(V_TIMER1_COMPB); sim_oc_match(2, cb, 0, 1); }
    if(c == top){ TCNT1 = 0; if(c == 0xFFFF) sim_event(V_TIMER1_OVF); }
    else TCNT1 = (uint16_t)(c + 1);
    if(c == 0xFFFF && top != 0xFFFF) sim_event(V_TIMER1_OVF);
  } else if(kind == 1){
    if(c >= top){ TCNT1 = 0; sim_event(V_TIMER1_OVF); sim_oc_bottom(1, ca); sim_oc_bottom(2, cb); }
    else { if(c == OCR1A){ sim_event(V_TIMER1_COMPA); sim_oc_match(1, ca, 1, 1); }
           if(c == OCR1B){ sim_event(V_TIMER1_COMPB); sim_oc_match(2, cb, 1, 1); }
           TCNT1 = (uint16_t)(c + 1); }
  } else {
    int *up = &sim_tm[1].up;
    if(*up){ if(c >= top){ *up = 0; TCNT1 = (uint16_t)(top - 1); } else TCNT1 = (uint16_t)(c + 1); }
    else { if(c == 0){ *up = 1; TCNT1 = 1; } else TCNT1 = (uint16_t)(c - 1); }
    if(TCNT1 == OCR1A){ sim_event(V_TIMER1_COMPA); sim_oc_match(1, ca, 2, *up); }
    if(TCNT1 == OCR1B){ sim_event(V_TIMER1_COMPB); sim_oc_match(2, cb, 2, *up); }
    if(TCNT1 == 0) sim_event(V_TIMER1_OVF);
  }
}
static int    sim_duty_pin = -1;
static double sim_duty_hi = 0;
void run_ms(double ms){
  uint64_t end = sim_cyc + (uint64_t)(ms * SIM_F_CPU / 1000.0 + 0.5);
  sim_after(); sim_tsync();
  for(;;){
    uint64_t t = end; int any = 0;
    for(int k = 0; k < 3; k++) if(sim_tm[k].div && sim_tm[k].next <= t){ t = sim_tm[k].next; any = 1; }
    if(sim_duty_pin >= 0 && pin_level(sim_duty_pin)) sim_duty_hi += (double)(t - sim_cyc);
    sim_cyc = t;
    if(!any) break;
    for(int k = 0; k < 3; k++) if(sim_tm[k].div && sim_tm[k].next == t){
      if(k == 1) sim_tick16(); else sim_tick8(k);
      sim_tm[k].next += (uint64_t)sim_tm[k].div;
    }
    sim_after(); sim_tsync();
  }
}
double sim_time_ms(void){ return (double)sim_cyc * 1000.0 / SIM_F_CPU; }
double pin_duty(int pin, double ms){        /* доля времени с уровнем 1 на выводе за ms */
  sim_duty_pin = pin; sim_duty_hi = 0; uint64_t c0 = sim_cyc;
  run_ms(ms);
  sim_duty_pin = -1;
  return sim_cyc > c0 ? sim_duty_hi / (double)(sim_cyc - c0) : 0;
}

/* ---------------- внешний мир */
void pin_ext(int pin, int lvl){ sim_ext_lvl[pin] = lvl; sim_after(); }
void pin_wire(int a, int b){ if(sim_nw < SIM_MAXW){ sim_wa[sim_nw] = a; sim_wb[sim_nw] = b; sim_nw++; } sim_after(); }
void pin_unwire(int a, int b){
  for(int w = 0; w < sim_nw; w++)
    if((sim_wa[w] == a && sim_wb[w] == b) || (sim_wa[w] == b && sim_wb[w] == a)){
      sim_wa[w] = sim_wa[sim_nw - 1]; sim_wb[w] = sim_wb[sim_nw - 1]; sim_nw--; break; }
  sim_after();
}
/* кнопка на землю с дребезгом: 6 отскоков, после каждого T0(1) (в программах курса ~2 мс) */
void btn_press(int pin){ for(int i = 0; i < 6; i++){ pin_ext(pin, (i & 1) ? -1 : 0); T0(1); } pin_ext(pin, 0); T0(150); }
void btn_release(int pin){ for(int i = 0; i < 6; i++){ pin_ext(pin, (i & 1) ? 0 : -1); T0(1); } pin_ext(pin, -1); T0(150); }
void btn_tap(int pin){ btn_press(pin); btn_release(pin); }
void btn_down(int pin){ pin_ext(pin, 0); }      /* без дребезга и без ожидания */
void btn_up(int pin){ pin_ext(pin, -1); }

/* клавиатура: нажатая клавиша = провод между строкой и столбцом */
static int sim_krow[4] = {-1, -1, -1, -1}, sim_kcol[4] = {-1, -1, -1, -1}, sim_kr = -1, sim_kc = -1;
void kbd_pins(const int rows[4], const int cols[4]){ for(int i = 0; i < 4; i++){ sim_krow[i] = rows[i]; sim_kcol[i] = cols[i]; } }
static void sim_kset(int on){
  if(sim_kr < 0) return;
  pin_unwire(sim_krow[sim_kr], sim_kcol[sim_kc]);
  if(on) pin_wire(sim_krow[sim_kr], sim_kcol[sim_kc]);
}
void kbd_down(int r, int c){
  if(sim_kr >= 0){ sim_kset(0); }
  sim_kr = r; sim_kc = c;
  for(int i = 0; i < 6; i++){ sim_kset(!(i & 1)); T0(1); }
  sim_kset(1); sim_after();
}
void kbd_up(void){
  if(sim_kr < 0) return;
  for(int i = 0; i < 6; i++){ sim_kset(i & 1); T0(1); }
  sim_kset(0); sim_kr = sim_kc = -1; sim_after();
}
void kbd_tap(int r, int c, int hold){ kbd_down(r, c); T0(hold); kbd_up(); T0(40); }

/* ---------------- индикатор: что реально выводит развёртка */
static int sim_dig[4] = {1*8+4, 1*8+5, 1*8+6, 1*8+7};
void disp_digits(int d0, int d1, int d2, int d3){ sim_dig[0] = d0; sim_dig[1] = d1; sim_dig[2] = d2; sim_dig[3] = d3; }
int seg2dig(unsigned char code){
  static const unsigned char k[16] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F,
                                      0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71};
  code &= 0x7F;
  if(code == 0x00) return -1;
  if(code == 0x40) return -2;
  for(int i = 0; i < 16; i++) if(k[i] == code) return i;
  return -3;
}
static int sim_seg_byte(void){ int v = 0; for(int b = 0; b < 8; b++) if(sim_drive(3 * 8 + b) == 1) v |= 1 << b; return v; }
static int sim_only(int i){                  /* включён ровно разряд i (0 на его выводе, 1 на остальных) */
  for(int k = 0; k < 4; k++){ if(sim_dig[k] < 0) continue;
    int d = sim_drive(sim_dig[k]);
    if(k == i ? d != 0 : d != 1) return 0; }
  return sim_dig[i] >= 0;
}
int disp_code(int i){                        /* код сегментов разряда i вместе с битом PD7 (точка; в варианте А
                                                клавиатуры это столбец, маскируйте 0x7F); -1, если разряд не зажёгся */
  /* всегда хотя бы один шаг развёртки: разряд должен быть перерисован заново, а не остаться с прошлого раза */
  for(int n = 0; n < 8; n++){ T0(1); if(sim_only(i)) return sim_seg_byte(); }
  return -1;
}
int disp_digit(int i){ int c = disp_code(i); return c < 0 ? -4 : seg2dig((unsigned char)c); }
int disp_num(void){
  int v = 0;
  for(int i = 0; i < 4; i++){ int d = disp_digit(i); if(d == -1) d = 0; if(d < 0 || d > 9) return -1; v = v * 10 + d; }
  return v;
}

/* ---------------- запуск */
static jmp_buf sim_jb;
static int sim_loop_left = 0, sim_loop_k = 0;
typedef void (*hook_t)(int k, double ms);
static hook_t sim_hook = 0;
void _delay_ms(double ms){
  run_ms(ms);
  if(sim_hook) sim_hook(sim_loop_k, ms);
  sim_loop_k++;
  if(sim_loop_left && --sim_loop_left == 0) longjmp(sim_jb, 1);
}
void _delay_us(double us){ _delay_ms(us / 1000.0); }
static void sim_init_prev(void){ sim_prev_set = 0; }
void sim_start(void){
  sim_init_prev();
  prog_main();
  sim_check_vectors("после main");
  sim_after(); sim_tsync();
}
void sim_loop(int ndelays, hook_t hook){     /* для программ с непустым while(1) и _delay_ms */
  sim_init_prev();
  sim_hook = hook; sim_loop_k = 0; sim_loop_left = ndelays;
  if(setjmp(sim_jb) == 0) prog_main();
  sim_hook = 0; sim_loop_left = 0;
  sim_check_vectors("в цикле main");
}
int sim_done(const char *name){
  sim_check_vectors("в конце теста");
  if(sim_conflicts) printf("  (внимание: %d раз два выхода с разными уровнями на одном проводе)\n", sim_conflicts);
  printf("%-8s %s\n", sim_fails ? "FAILED" : "ok", name);
  return sim_fails ? 1 : 0;
}
#endif
