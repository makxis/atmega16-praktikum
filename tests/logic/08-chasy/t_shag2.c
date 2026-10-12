/* Глава 8, шаг 2 (shag2.c): часы MM:SS; Timer1 даёт ровно 1 прерывание в секунду. */
#include "sim.h"
#include "08-chasy/shag2.c"
/* Перед чтением индикатора - полный круг развёртки (4 вызова Timer0): после ручного T1() или правки
   переменных разряд, который горит в этот момент, иначе показал бы старый код. */
static int num(void){ T0(4); return disp_num(); }
int main(void){
  sim_start();
  CHECK(num()==0, "старт: 00:00");
  T1(61); CHECK(num()==101, "61 с = 01:01");
  T1(59*60+59-61); CHECK(num()==5959, "59:59");
  T1(1); CHECK(num()==0, "59:59 -> 00:00");
  int ok = 1;
  for(int i = 0; i < 8; i++){ unsigned char idx = i1; T0(1);
    if(PORTD != (unsigned char)Ind[idx] || PORTB != (unsigned char)Mas[idx]) ok = 0; }
  CHECK(ok, "развёртка: за вызов Timer0 горит один разряд, PORTD = Ind[i1], PORTB = Mas[i1]");
  isr_calls_reset(); run_ms(10500);
  CHECK(isr_calls("TIMER1_COMPA")==10, "10,5 с модельного времени: ровно 10 прерываний Timer1 (1 в секунду)");
  long n0 = isr_calls("TIMER0_OVF");
  CHECK(n0 >= 5126 && n0 <= 5127, "Timer0: переполнение каждые 2,048 мс (488 в секунду)");
  CHECK(num()==10, "часы по модельному времени: 00:10");
  return sim_done("08/shag2");
}
