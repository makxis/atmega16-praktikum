/* Глава 9, задание 3 (edge.c = Knopka.c с переделкой): одно нажатие = ровно +1. */
#include "sim.h"
#include "09-knopka/edge.c"
#define BTN PIN('C', 1)
/* Перед чтением индикатора - полный круг развёртки (4 вызова Timer0): после ручного T1() или правки
   переменных разряд, который горит в этот момент, иначе показал бы старый код. */
static int num(void){ T0(4); return disp_num(); }
int main(void){
  sim_start();
  CHECK(pin_pullup(BTN), "подтяжка на PC1");
  T0(500); CHECK(num()==0, "старт 0000");
  for(int i = 0; i < 20; i++){ btn_press(BTN); T0(500); btn_release(BTN); }
  CHECK(num()==20, "20 нажатий с дребезгом и удержанием по 1 с = ровно 0020");
  for(int i = 0; i < 9979; i++) btn_tap(BTN);
  CHECK(num()==9999, "9999 нажатий = 9999");
  btn_tap(BTN); CHECK(num()==0, "после 9999 -> 0000");
  return sim_done("09/edge");
}
