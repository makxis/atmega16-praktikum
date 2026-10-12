/* Глава 6, задание 4 (int2_led.c): кнопка на PB2, INT2 по фронту (отпускание) переключает светодиод PB4. */
#include "sim.h"
#include "06-preryvaniya/int2_led.c"
#define BTN PIN('B', 2)
#define L1  PIN('B', 4)
static long c0 = -1, c_down = -1, c_up = -1, c_two = -1;
static int l_down = -1, l_up = -1, l_two = -1;
static void hook(int k, double ms){
  (void)ms;
  if(k == 0) c0 = isr_calls("INT2");
  if(k == 2) btn_down(BTN);                         /* нажали: спад, INT2 молчит */
  if(k == 3){ c_down = isr_calls("INT2"); l_down = pin_out(L1); btn_up(BTN); }  /* отпустили: фронт */
  if(k == 4){ c_up = isr_calls("INT2"); l_up = pin_out(L1); btn_down(BTN); }
  if(k == 5) btn_up(BTN);
  if(k == 6){ c_two = isr_calls("INT2"); l_two = pin_out(L1); }
}
int main(void){
  sim_loop(8, hook);
  CHECK(c0 == 0, "после настройки ложного вызова INT2 нет (флаг INTF2 сброшен до разрешения)");
  CHECK(c_down == 0 && l_down == 1, "нажатие (спад) INT2 не вызывает");
  CHECK(c_up == 1 && l_up == 0, "отпускание (фронт): INT2, светодиод 1 горит");
  CHECK(c_two == 2 && l_two == 1, "второе нажатие-отпускание: светодиод 1 снова погашен");
  CHECK(pin_pullup(BTN), "PB2: подтяжка на месте (обработчик её не затирает)");
  CHECK(MCUCSR & (1 << ISC2), "MCUCSR: ISC2=1 (нарастающий фронт)");
  return sim_done("06/int2_led");
}
