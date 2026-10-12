/* Глава 6, задание 3, вариант (tests/mutants/06-flag-clear.json): строка GIFR=(1<<INTF0) перед sei()
   раскомментирована. Флаг сброшен до sei(), поэтому нажатия за время ожидания забыты. */
#include "sim.h"
#include "06-preryvaniya/flag__clear.c"
#define BTN PIN('D', 2)
static long c_sei = -1, c_more = -1;
static void hook(int k, double ms){
  (void)ms;
  if(k == 10) btn_down(BTN);
  if(k == 12) btn_up(BTN);
  if(k == 50){ c_sei = isr_calls("INT0"); btn_down(BTN); }
  if(k == 51){ c_more = isr_calls("INT0"); btn_up(BTN); }
}
int main(void){
  sim_loop(53, hook);
  CHECK(c_sei == 0 && pin_out(PIN('B', 4)) == 0, "флаг сброшен до sei(): старое нажатие забыто");
  CHECK(c_more == 1, "новое нажатие после sei() срабатывает");
  return sim_done("06/flag__clear (вариант задания)");
}
