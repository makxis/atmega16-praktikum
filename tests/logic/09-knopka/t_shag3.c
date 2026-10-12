/* Глава 9, шаг 3 (shag3.c): считается момент нажатия, удержание = +1, после 9999 -> 0000. */
#include "sim.h"
#include "09-knopka/shag3.c"
#define BTN PIN('C', 1)
/* Перед чтением индикатора - полный круг развёртки (4 вызова Timer0): после ручного T1() или правки
   переменных разряд, который горит в этот момент, иначе показал бы старый код. */
static int num(void){ T0(4); return disp_num(); }
int main(void){
  sim_start();
  T0(200); CHECK(num()==0, "старт 0000");
  for(int i = 0; i < 25; i++){ btn_press(BTN); T0(1000); btn_release(BTN); }
  CHECK(num()==25, "25 нажатий с дребезгом и удержанием по 2 с = 0025");
  btn_down(BTN); run_ms(3000); btn_up(BTN); T0(50);
  CHECK(num()==26, "удержание 3 с модельного времени = ровно +1");
  for(int i = 0; i < 9973; i++) btn_tap(BTN);
  CHECK(num()==9999, "9999 нажатий = 9999");
  btn_tap(BTN); CHECK(num()==0, "после 9999 -> 0000");
  return sim_done("09/shag3");
}
