/* Глава 8, шаг 1 (shag1.c): show() и десятки секунд, секунды идут 00..59. */
#include "sim.h"
#include "08-chasy/shag1.c"
/* Перед чтением индикатора - полный круг развёртки (4 вызова Timer0): после ручного T1() или правки
   переменных разряд, который горит в этот момент, иначе показал бы старый код. */
static int num(void){ T0(4); return disp_num(); }
int main(void){
  sim_start();
  CHECK(num()==0, "старт: 0000 (show() в main)");
  CHECK(OCR1A==31249, "OCR1A=31249");
  T1(59); CHECK(num()==59, "59 с: 0059");
  T1(1);  CHECK(num()==0, "60 с: снова 0000 (минут на этом шаге нет)");
  isr_calls_reset(); run_ms(2500);
  CHECK(isr_calls("TIMER1_COMPA")==2, "2,5 с модельного времени: ровно 2 прерывания Timer1");
  CHECK(num()==2, "через 2,5 с: 0002");
  return sim_done("08/shag1");
}
