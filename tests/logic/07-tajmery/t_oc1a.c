/* Глава 7 (oc1a.c): Timer1 в CTC сам переключает OC1A (PD5) каждые 0,5 с, без прерываний. */
#include "sim.h"
#include "07-tajmery/oc1a.c"
#define LED PIN('D', 5)
int main(void){
  sim_start();
  CHECK(pin_out(LED) == 0, "PD5 - выход (DDRD), на старте OC1A = 0: светодиод горит");
  CHECK(OCR1A == 15624, "OCR1A=15624");
  CHECK(TIMSK == 0, "прерывания не нужны: TIMSK=0");
  int prev = pin_out(LED), changes = 0; double t[8] = {0};
  for(int ms = 0; ms < 3000; ms++){
    run_ms(1);
    int v = pin_out(LED);
    if(v != prev){ if(changes < 8) t[changes] = sim_time_ms(); changes++; prev = v; }
  }
  CHECK(changes == 6, "за 3 с OC1A переключился 6 раз: мигание 1 Гц");
  CHECK(t[0] > 499.9 && t[0] < 501.1 && t[1] > 999.9 && t[1] < 1001.1, "переключения через 0,5 с и 1,0 с");
  double d = pin_duty(LED, 2000);
  CHECK(d > 0.499 && d < 0.501, "меандр: половину времени 1, половину 0");
  CHECK(isr_calls("TIMER1_COMPA") == 0, "обработчик не вызывался");
  return sim_done("07/oc1a");
}
