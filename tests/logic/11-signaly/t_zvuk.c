/* Глава 11: zvuk.c. Пищалка: пока нажата PC1, Timer2 в CTC переключает OC2 = PD7, меандр 1000 Гц. */
#include "sim.h"
#include "11-signaly/zvuk.c"
#define BUZ PIN('D', 7)
#define BTN PIN('C', 1)
static int periods(double ms){                          /* число фронтов из 0 в 1 на PD7 за ms */
  int n = 0, prev = pin_level(BUZ);
  for(int i = 0; i < (int)(ms / 0.02); i++){ run_ms(0.02); int l = pin_level(BUZ); if(l && !prev) n++; prev = l; }
  return n;
}
int main(void){
  sim_start();
  CHECK(OCR2 == 124, "OCR2 = 124");
  CHECK((TCCR2 & ((1<<WGM20)|(1<<WGM21)|(1<<CS22)|(1<<CS21)|(1<<CS20))) == ((1<<WGM21)|(1<<CS21)|(1<<CS20)),
        "Timer2: режим CTC, делитель 32 (CS21 и CS20)");
  CHECK(pin_pullup(BTN), "подтяжка на PC1");
  run_ms(20);
  CHECK(!(TCCR2 & (1<<COM20)) && periods(50) == 0 && pin_level(BUZ) == 0, "без кнопки тишина: OC2 отключён, на PD7 0");
  btn_down(BTN); run_ms(5);
  CHECK(TCCR2 & (1<<COM20), "кнопка нажата: OC2 подключён");
  int n = periods(100);
  printf("  (за 100 мс на PD7 %d периодов)\n", n);
  CHECK(n >= 99 && n <= 101, "нажата: на PD7 1000 Гц (100 периодов за 100 мс)");
  btn_up(BTN); run_ms(5);
  CHECK(!(TCCR2 & (1<<COM20)) && periods(50) == 0 && pin_level(BUZ) == 0, "отпустили: снова тишина");
  return sim_done("11/zvuk");
}
