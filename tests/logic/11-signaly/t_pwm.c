/* Глава 11: pwm.c. Яркость светодиода (горит от 0) через инверсный Fast PWM на OC0 = PB3.
   Проверяем: биты TCCR0 и OCR0; PC1 ярче, PC0 тусклее по 10 %, нажатие с удержанием = один шаг;
   доля времени, когда светодиод горит (на PB3 0), совпадает с процентами на индикаторе;
   0 % - настоящий ноль (OC0 отключён, PB3 = 1), 100 % - постоянный 0; индикация идёт 488 раз в секунду. */
#include "sim.h"
#include "11-signaly/pwm.c"
#define LED PIN('B', 3)
#define UP  PIN('C', 1)
#define DN  PIN('C', 0)
static double dabs(double x){ return x < 0 ? -x : x; }
static double lit(void){                               /* доля времени, когда горит (на PB3 0) */
  run_ms(5);                                            /* дать закончиться периоду со старым OCR0 */
  return 1.0 - pin_duty(LED, 100);
}
int main(void){
  sim_start();
  T0(8);
  CHECK(TCCR0 == ((1<<WGM01)|(1<<WGM00)|(1<<COM01)|(1<<COM00)|(1<<CS01)|(1<<CS00)),
        "TCCR0: Fast PWM, инверсный выход OC0, делитель 64 (0x7B)");
  CHECK(OCR0 == 127 && disp_num() == 50, "старт: 50 %, OCR0 = 127");
  CHECK(dabs(lit() - 0.5) < 0.01, "50 %: светодиод горит половину периода");
  btn_tap(UP);
  CHECK(level == 6 && OCR0 == 153 && disp_num() == 60, "PC1: 60 %, OCR0 = 153");
  btn_press(UP); T0(1000); btn_release(UP);
  CHECK(level == 7 && disp_num() == 70, "PC1 с удержанием 2 с: ровно один шаг, 70 %");
  for(int i = 0; i < 5; i++) btn_tap(UP);
  CHECK(level == 10 && OCR0 == 255 && disp_num() == 100, "до упора: 100 %, OCR0 = 255, дальше не растёт");
  CHECK(lit() > 0.999, "100 %: на PB3 всё время 0, светодиод горит постоянно");
  int bad = 0;
  for(int l = 9; l >= 1; l--){
    btn_tap(DN);
    double want = (OCR0 + 1) / 256.0, got = lit();
    if(level != l || disp_num() != l * 10 || dabs(got - want) > 0.01 || dabs(got - l / 10.0) > 0.02){
      printf("  уровень %d: level=%d, индикатор %d, горит %.3f (ждали %.3f)\n", l, level, disp_num(), got, want); bad++; }
  }
  CHECK(bad == 0, "90..10 %: светодиод горит (OCR0+1)/256 периода, это уровень с точностью 2 %");
  btn_tap(DN);
  CHECK(level == 0 && disp_num() == 0, "PC0 до упора: 0 %");
  CHECK(!(TCCR0 & ((1<<COM01)|(1<<COM00))), "0 %: OC0 отключён от вывода (COM01 = COM00 = 0)");
  CHECK(pin_out(LED) == 1 && lit() < 0.001, "0 %: на PB3 всё время 1 (бит PORTB), светодиод не горит совсем");
  btn_tap(DN);
  CHECK(level == 0, "ниже 0 % не уходит");
  btn_tap(UP);
  CHECK(level == 1 && (TCCR0 & (1<<COM01)) && (TCCR0 & (1<<COM00)) && OCR0 == 25, "PC1 после 0 %: 10 %, OC0 снова подключён");
  isr_calls_reset(); run_ms(1000);
  CHECK(isr_calls("TIMER0_OVF") >= 487 && isr_calls("TIMER0_OVF") <= 489, "индикация в Fast PWM: 488 прерываний в секунду");
  return sim_done("11/pwm");
}
