/* Глава 9, доработка (two.c): PC1 прибавляет, PC0 отнимает, после 0000 идёт 9999. */
#include "sim.h"
#include "09-knopka/two.c"
#define UP PIN('C', 1)
#define DN PIN('C', 0)
/* Перед чтением индикатора - полный круг развёртки (4 вызова Timer0): после ручного T1() или правки
   переменных разряд, который горит в этот момент, иначе показал бы старый код. */
static int num(void){ T0(4); return disp_num(); }
int main(void){
  sim_start();
  CHECK(pin_pullup(UP) && pin_pullup(DN), "подтяжка на PC1 и PC0");
  btn_tap(DN); CHECK(num()==9999, "0000 - 1 = 9999");
  btn_tap(UP); CHECK(num()==0, "9999 + 1 = 0000");
  for(int i = 0; i < 15; i++) btn_tap(UP);
  btn_tap(DN); CHECK(num()==14, "15 вверх, 1 вниз = 0014");
  t1 = 0; t2 = 0; t3 = 0; t4 = 1; show();
  btn_tap(DN); CHECK(num()==999, "1000 - 1 = 0999 (заём через три разряда)");
  btn_press(UP); T0(1000); btn_release(UP); CHECK(num()==1000, "удержание PC1 = ровно +1");
  reset(); CHECK(num()==0, "reset() (вариант со сбросом): 0000");
  return sim_done("09/two");
}
