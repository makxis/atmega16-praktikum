/* Глава 6, задание 2 (count.c): INT0 считает спады на PD2, main показывает счёт на PB4..PB7 (горит от 0). */
#include "sim.h"
#include "06-preryvaniya/count.c"
#define BTN PIN('D', 2)
static int shown(void){                    /* двоичное число на светодиодах: горит (0) = бит 1 */
  int v = 0;
  for(int i = 0; i < 4; i++) if(pin_out(PIN('B', 4 + i)) == 0) v |= 1 << i;
  return v;
}
static int s0 = -1, s1 = -1, s2 = -1, s3 = -1, ok_disp = 1;
static long c1 = -1, c2 = -1, c3 = -1;
static void hook(int k, double ms){
  (void)ms;
  if(shown() != (cnt & 15)) ok_disp = 0;
  if(k == 2){ s0 = shown(); btn_down(BTN); }                       /* чистое нажатие: +1 */
  if(k == 4){ s1 = shown(); c1 = cnt; btn_up(BTN); }
  if(k == 6){ pin_ext(BTN, 0); pin_ext(BTN, -1); pin_ext(BTN, 0); pin_ext(BTN, -1); pin_ext(BTN, 0); } /* 3 спада */
  if(k == 8){ s2 = shown(); c2 = cnt;                              /* отпускание с дребезгом: ещё 2 спада */
              pin_ext(BTN, -1); pin_ext(BTN, 0); pin_ext(BTN, -1); pin_ext(BTN, 0); pin_ext(BTN, -1); }
  if(k == 10){ s3 = shown(); c3 = cnt; }
}
int main(void){
  sim_loop(12, hook);
  CHECK(s0 == 0, "на старте счёт 0: все светодиоды погашены");
  CHECK(c1 == 1 && s1 == 1, "одно чистое нажатие: счёт 1, горит только светодиод PB4");
  CHECK(c2 == 4 && s2 == 4, "нажатие с тремя отскоками: счёт сразу 4 (0100)");
  CHECK(c3 == 6 && s3 == 6, "отпускание с дребезгом добавило ещё 2: счёт 6 (0110)");
  CHECK(ok_disp, "светодиоды всегда показывают младшие 4 бита cnt");
  CHECK(pin_pullup(BTN), "PD2: вход с подтяжкой");
  return sim_done("06/count");
}
