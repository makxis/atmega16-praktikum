/* Глава 9, доработка (clockbtn.c): часы идут сами, PC1 добавляет минуту. */
#include "sim.h"
#include "09-knopka/clockbtn.c"
#define BTN PIN('C', 1)
/* Перед чтением индикатора - полный круг развёртки (4 вызова Timer0): после ручного T1() или правки
   переменных разряд, который горит в этот момент, иначе показал бы старый код. */
static int num(void){ T0(4); return disp_num(); }
int main(void){
  sim_start();
  T1(30); btn_tap(BTN); CHECK(num()==130, "00:30 + минута = 01:30");
  for(int i = 0; i < 59; i++) btn_tap(BTN);
  CHECK(num()==30, "ещё 59 минут: 60:30 -> 00:30");
  btn_press(BTN); T0(1000); btn_release(BTN); CHECK(num()==130, "удержание кнопки = ровно +1 минута");
  isr_calls_reset(); run_ms(2500);
  CHECK(isr_calls("TIMER1_COMPA")==2 && num()==132, "часы идут сами: через 2,5 с 01:32");
  return sim_done("09/clockbtn");
}
