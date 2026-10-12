/* Глава 6 (int0.c): кнопка на PD2 через INT0 переключает светодиод PB4, основной цикл мигает PB5. */
#include "sim.h"
#include "06-preryvaniya/int0.c"
#define BTN PIN('D', 2)
#define L1  PIN('B', 4)
#define L2  PIN('B', 5)
static int l1[16], l2[16], n = 0;
static long c_press = -1, c_release = -1, c_bounce = -1;
static void hook(int k, double ms){
  (void)ms;
  if(n < 16){ l1[n] = pin_out(L1); l2[n] = pin_out(L2); n++; }
  if(k == 2) btn_down(BTN);                         /* нажали посреди мигания (без дребезга) */
  if(k == 3){ c_press = isr_calls("INT0"); btn_up(BTN); }
  if(k == 4){ c_release = isr_calls("INT0");        /* нажатие с дребезгом: 0, 1, 0, 1, 0 = три спада */
              pin_ext(BTN, 0); pin_ext(BTN, -1); pin_ext(BTN, 0); pin_ext(BTN, -1); pin_ext(BTN, 0); }
  if(k == 5){ c_bounce = isr_calls("INT0"); btn_up(BTN); }
}
int main(void){
  sim_loop(8, hook);
  int blink = (n == 8);
  for(int k = 0; k < n; k++) if(l2[k] != (k & 1)) blink = 0;
  CHECK(blink, "светодиод 2 (PB5) мигает: полсекунды горит (0), полсекунды нет (1)");
  CHECK(l1[0] == 1 && l1[1] == 1 && l1[2] == 1, "до нажатия светодиод 1 (PB4) погашен");
  CHECK(c_press == 1 && l1[3] == 0, "нажатие: INT0 вызван сразу, светодиод 1 горит");
  CHECK(c_release == 1 && l1[4] == 0, "отпускание (фронт с 0 на 1) INT0 не вызывает: прерывание по спаду");
  CHECK(c_bounce == 4 && l1[5] == 1, "нажатие с дребезгом: три спада = три вызова INT0");
  CHECK(pin_pullup(BTN), "PD2: вход с подтяжкой");
  CHECK((MCUCR & 0x03) == 0x02, "MCUCR: ISC01=1, ISC00=0 (спад)");
  CHECK(GICR & (1 << INT0), "GICR: INT0 разрешён");
  return sim_done("06/int0");
}
