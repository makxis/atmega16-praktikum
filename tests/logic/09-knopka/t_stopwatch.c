/* Глава 9, доработка (stopwatch.c): секундомер, PC1 старт/стоп, PC0 сброс. */
#include "sim.h"
#include "09-knopka/stopwatch.c"
#define SS PIN('C', 1)
#define RS PIN('C', 0)
/* Перед чтением индикатора - полный круг развёртки (4 вызова Timer0): после ручного T1() или правки
   переменных разряд, который горит в этот момент, иначе показал бы старый код. */
static int num(void){ T0(4); return disp_num(); }
int main(void){
  sim_start();
  CHECK(!(TCCR1B & (1<<CS12)) && num()==0, "старт: 00:00, Timer1 стоит");
  run_ms(3000); CHECK(num()==0 && isr_calls("TIMER1_COMPA")==0, "3 с без старта: стоит");
  btn_tap(SS); CHECK(TCCR1B & (1<<CS12), "PC1: пошёл");
  T1(75); btn_tap(SS); CHECK(!(TCCR1B & (1<<CS12)) && num()==115, "PC1: стоп на 01:15");
  TCNT1 = 1234; btn_tap(RS); CHECK(num()==0 && TCNT1==0, "PC0: сброс в 00:00, TCNT1 = 0");
  btn_tap(SS); run_ms(5500); btn_tap(SS);
  CHECK(num()==5, "старт, 5,5 с модельного времени, стоп: 00:05");
  run_ms(5000); CHECK(num()==5, "на паузе 5 с: всё ещё 00:05");
  btn_tap(SS); run_ms(600);
  CHECK(num()==6, "после паузы начатая секунда не потерялась: через 0,6 с уже 00:06");
  return sim_done("09/stopwatch");
}
