/* Глава 4, шаг 1 (shag1.c): светодиод PB4 повторяет кнопку PC1, опрос раз в 10 мс. */
#include "sim.h"
#include "04-porty/shag1.c"
#define BTN PIN('C', 1)
static int btn = 0;                     /* кнопка нажата с прошлого опроса */
static int bad_led = 0, bad_other = 0, bad_ms = 0, lit = 0;
static void hook(int k, double ms){     /* после каждой паузы _delay_ms */
  int led = pin_out(PIN('B', 4));       /* 0 = светодиод горит */
  if(led != (btn ? 0 : 1)) bad_led++;
  if(btn && led == 0) lit++;
  for(int i = 5; i < 8; i++) if(pin_out(PIN('B', i)) != 1) bad_other++;
  if(ms != 10) bad_ms++;
  if(k == 9){ btn_down(BTN); btn = 1; }
  if(k == 19){ btn_up(BTN); btn = 0; }
}
int main(void){
  sim_loop(30, hook);
  CHECK(pin_pullup(BTN), "PC1: вход с подтяжкой");
  CHECK(ACSR & (1 << ACD), "компаратор выключен (ACD = 1)");
  CHECK(bad_led == 0, "светодиод PB4 повторяет кнопку: нажата - горит (0), отпущена - погашен (1)");
  CHECK(lit == 10, "кнопка нажата 10 опросов: светодиод горит все 10");
  CHECK(bad_other == 0, "светодиоды PB5..PB7 всё время погашены (1)");
  CHECK(bad_ms == 0, "опрос раз в 10 мс");
  return sim_done("04/shag1");
}
