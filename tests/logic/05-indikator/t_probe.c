/* Глава 5, задание 2 (probe.c): программа-щуп, каждое нажатие PC1 - следующий бит PORTD. */
#include "sim.h"
#include "05-indikator/probe.c"
#define BTN PIN('C', 1)
static int code(int i){ T0(4); return disp_code(i); }
int main(void){
  sim_start();
  CHECK(pin_pullup(BTN), "PC1: вход с подтяжкой");
  CHECK(code(0) == 0x01 && code(1) == 0x01 && code(2) == 0x01 && code(3) == 0x01, "старт: во всех разрядах бит 0 (PD0)");
  T0(1000); CHECK(code(0) == 0x01, "кнопку не трогаем 2 с: щуп стоит на PD0");
  btn_tap(BTN); T0(400);
  CHECK(code(0) == 0x02 && code(3) == 0x02, "короткое нажатие (с дребезгом): PD1 во всех разрядах");
  int ok = 1;
  for(int i = 2; i <= 8; i++){ btn_tap(BTN); T0(400); if(code(1) != (1 << (i & 7))) ok = 0; }
  CHECK(ok, "нажатия 2..7: PD2..PD7 по очереди");
  CHECK(code(2) == 0x01, "8 нажатий: снова PD0");
  btn_press(BTN); T0(600); btn_release(BTN); T0(400);
  CHECK(nbit == 3 && code(0) == 0x08, "удержание ~1,5 с: щуп перескочил 3 раза (раз в 0,6 с), горит PD3");
  return sim_done("05/probe");
}
