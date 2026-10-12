/* Глава 6, задание 3 (flag.c): нажатия до sei() не вызывают INT0, но флаг INTF0 их помнит:
   сразу после sei() обработчик вызывается ровно один раз. */
#include "sim.h"
#include "06-preryvaniya/flag.c"
#define BTN PIN('D', 2)
#define L1  PIN('B', 4)
static long c_wait = -1, c_sei = -1, c_more = -1;
static int  l1_wait = -1, l1_sei = -1, fast = 1;
static void hook(int k, double ms){
  if(k < 50 && ms != 100) fast = 0;
  if(k == 10) btn_down(BTN);                        /* два нажатия за время ожидания */
  if(k == 12) btn_up(BTN);
  if(k == 20) btn_down(BTN);
  if(k == 22) btn_up(BTN);
  if(k == 49){ c_wait = isr_calls("INT0"); l1_wait = pin_out(L1); }
  if(k == 50){ c_sei = isr_calls("INT0"); l1_sei = pin_out(L1);
               btn_down(BTN); }                     /* после sei() нажатие срабатывает сразу */
  if(k == 51){ c_more = isr_calls("INT0"); btn_up(BTN); }
}
int main(void){
  sim_loop(53, hook);
  CHECK(fast, "5 секунд ожидания: шаги по 0,1 с");
  CHECK(c_wait == 0 && l1_wait == 1, "до sei() обработчик не вызывался, светодиод 1 погашен");
  CHECK(c_sei == 1 && l1_sei == 0, "после sei() отложенный INT0 вызван ровно один раз за два нажатия");
  CHECK(c_more == 2, "дальше каждое нажатие вызывает INT0 сразу");
  return sim_done("06/flag");
}
