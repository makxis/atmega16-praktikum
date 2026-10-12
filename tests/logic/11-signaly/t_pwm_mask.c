/* Мутант pwm.c с масками без "| 0x08" (tests/mutants/11-pwm-mask.json): пока OC0 подключён, бит PB3
   в PORTB не действует. Но на 0 % OC0 отключён, и развёртка пишет в PB3 ноль: светодиод горит полностью. */
#include "sim.h"
#include "11-signaly/pwm__mask.c"
#define LED PIN('B', 3)
int main(void){
  sim_start();
  for(int i = 0; i < 6; i++) btn_tap(PIN('C', 0));     /* 50 % -> 0 % */
  double lit = 1.0 - pin_duty(LED, 100);
  CHECK(level == 0 && lit > 0.9, "мутант пойман: на 0 % светодиод горит полностью");
  return sim_done("11/pwm мутант без | 0x08");
}
