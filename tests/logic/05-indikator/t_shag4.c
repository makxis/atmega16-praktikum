/* Глава 5, шаг 4 (shag4.c): правая цифра +1 раз в секунду, после 9 снова 0. */
#include "sim.h"
#include "05-indikator/shag4.c"
/* Перед чтением индикатора - полный круг развёртки: разряд, который горит сейчас, мог показать старый код. */
static int num(void){ T0(4); return disp_num(); }
int main(void){
  sim_start();
  CHECK(num() == 0, "старт: 0000");
  CHECK(OCR1A == 31249, "OCR1A=31249");
  isr_calls_reset(); run_ms(1000);
  CHECK(isr_calls("TIMER1_COMPA") == 1, "1 с модельного времени: ровно одно прерывание Timer1");
  long n0 = isr_calls("TIMER0_OVF");
  CHECK(n0 >= 487 && n0 <= 489, "1 с: ~488 переполнений Timer0");
  CHECK(num() == 1, "через 1 с: 0001");
  run_ms(8000); CHECK(num() == 9, "через 9 с: 0009");
  run_ms(1000); CHECK(num() == 0, "через 10 с: снова 0000 (переносов пока нет)");
  CHECK(isr_calls("TIMER1_COMPA") == 10, "за 10 с ровно 10 прерываний Timer1");
  return sim_done("05/shag4");
}
