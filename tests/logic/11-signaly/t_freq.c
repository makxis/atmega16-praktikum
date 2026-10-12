/* Глава 11: freq.c. Частотомер: фронты на PB2 (INT2) за окно Timer1 = 1 с.
   Проверяем: подтяжка PB2 не затирается индикацией; N фронтов между двумя вызовами Timer1 -> на
   индикаторе N (0..9999, больше - "----"); кнопка с дребезгом даёт несколько фронтов;
   свой меандр на PB3 (Timer0, Fast PWM) через перемычку PB3 - PB2 даёт 488 или 489 Гц. */
#include "sim.h"
#include "11-signaly/freq.c"
#define SIG PIN('B', 2)
static void edges(long n){ for(long i = 0; i < n; i++){ pin_ext(SIG, 0); pin_ext(SIG, 1); } pin_ext(SIG, -1); }
static int dashes(void){ for(int i = 0; i < 4; i++) if(disp_digit(i) != -2) return 0; return 1; }
int main(void){
  sim_start();
  CHECK(isr_calls("INT2") == 0, "настройка INT2 (ISC2, затем GIFR=(1<<INTF2)) не даёт ложного вызова на старте");
  T0(8);
  CHECK(disp_num() == 0, "старт: 0000");
  CHECK(pin_pullup(SIG), "подтяжка PB2 включена и индикация её не затирает (маски с | 0x04)");
  CHECK(TCCR0 == ((1<<WGM01)|(1<<WGM00)|(1<<COM01)|(1<<CS01)|(1<<CS00)), "Timer0: Fast PWM, прямой выход OC0, делитель 64");
  CHECK(OCR0 == 127 && OCR1A == 31249 && (MCUCSR & (1<<ISC2)) && (GICR & (1<<INT2)), "OCR0=127, OCR1A=31249, INT2 по фронту разрешён");
  T1(1);                                                /* закрываем первое окно */
  static const long ns[] = {0, 1, 7, 488, 1000, 9999};
  int bad = 0;
  for(unsigned k = 0; k < sizeof ns / sizeof ns[0]; k++){
    edges(ns[k]); T1(1);
    if(disp_num() != ns[k]){ printf("  %ld фронтов: на индикаторе %d\n", ns[k], disp_num()); bad++; }
  }
  CHECK(bad == 0, "N фронтов за окно -> на индикаторе N (0, 1, 7, 488, 1000, 9999)");
  edges(12345); T1(1);
  CHECK(dashes(), "12345 фронтов за окно: ----");
  T1(1);
  CHECK(disp_num() == 0, "окно без сигнала: 0000 (счёт начинается заново)");
  btn_tap(SIG); T1(1);
  CHECK(disp_num() > 1, "кнопка на PB2: одно нажатие с дребезгом даёт несколько фронтов");
  printf("  (кнопка в модели: одно нажатие = %d фронтов)\n", disp_num());
  isr_calls_reset();
  pin_wire(PIN('B', 3), SIG);                           /* перемычка PB3 - PB2 */
  run_ms(1000); run_ms(1000); run_ms(1000);
  long n0 = isr_calls("TIMER0_OVF"), n1 = isr_calls("TIMER1_COMPA"), n2 = isr_calls("INT2");
  int f = disp_num();
  CHECK(f == 488 || f == 489, "перемычка PB3 - PB2: 488 или 489 Гц");
  printf("  (перемычка PB3 - PB2: %d Гц; за 3 с TIMER0_OVF %ld, TIMER1_COMPA %ld, INT2 %ld)\n",
         f, n0, n1, n2);
  CHECK(n1 == 3, "Timer1: ровно одно окно в секунду");
  CHECK(n0 >= 1463 && n0 <= 1466, "индикация в Fast PWM: 488 прерываний в секунду, как в Normal");
  return sim_done("11/freq");
}
