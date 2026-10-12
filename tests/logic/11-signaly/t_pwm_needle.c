/* Мутант pwm.c без отключения OC0 на 0 % (tests/mutants/11-pwm-needle.json): на 0 % стоит OCR0 = 0,
   а инверсный Fast PWM при OCR0 = 0 всё равно даёт 0 в течение 1 тика из 256: светодиод чуть светится. */
#include "sim.h"
#include "11-signaly/pwm__needle.c"
#define LED PIN('B', 3)
int main(void){
  sim_start();
  for(int i = 0; i < 6; i++) btn_tap(PIN('C', 0));     /* 50 % -> 0 % */
  double lit = 1.0 - pin_duty(LED, 100);
  printf("  (мутант: на 0 %% светодиод горит %.4f периода, 1/256 = %.4f)\n", lit, 1 / 256.0);
  CHECK(level == 0 && lit > 0.003, "мутант пойман: на 0 % светодиод всё равно горит 1/256 периода");
  return sim_done("11/pwm мутант OCR0=0 на 0 %");
}
