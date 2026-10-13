/* Глава 1, щуп уровней (shchup.c): PB4 = 0, PB5 = 1, PB6 повторяет PC1 (с подтяжкой), PB7 повторяет PC0 (без). */
#include "sim.h"
#define while(c) for(int sim_once_ = 0; sim_once_ < 1 && (c); sim_once_++)   /* один проход цикла за вызов */
#include "01-oborudovanie/shchup.c"
#undef while
int main(void){
  sim_start();                                   /* ничего не подключено */
  CHECK(pin_out(PIN('B', 4)) == 0 && pin_out(PIN('B', 5)) == 1, "PB4 всё время 0, PB5 всё время 1");
  CHECK(pin_pullup(PIN('C', 1)) && !pin_pullup(PIN('C', 0)), "PC1 вход с подтяжкой, PC0 без подтяжки");
  CHECK(pin_out(PIN('B', 6)) == 1, "PC1 отпущен (подтяжка, 1): светодиод на PB6 погашен");
  btn_down(PIN('C', 1)); sim_start();
  CHECK(pin_out(PIN('B', 6)) == 0, "кнопка на PC1 нажата (0): светодиод на PB6 горит");
  btn_up(PIN('C', 1));
  pin_ext(PIN('C', 0), 1); sim_start();
  CHECK(pin_out(PIN('B', 7)) == 1, "на PC0 1: светодиод на PB7 погашен");
  pin_ext(PIN('C', 0), 0); sim_start();
  CHECK(pin_out(PIN('B', 7)) == 0, "на PC0 0: светодиод на PB7 горит");
  CHECK(pin_out(PIN('B', 6)) == 1, "PB6 по-прежнему повторяет PC1 (1)");
  return sim_done("01/shchup");
}
