/* Глава 7, задание 2 (oc0.c): Timer0 CTC, делитель 1024, OCR0=255: OC0 (PB3) переключается каждые 32,768 мс. */
#include "sim.h"
#include "07-tajmery/oc0.c"
#define LED PIN('B', 3)
int main(void){
  sim_start();
  CHECK(OCR0 == 255, "OCR0=255");
  CHECK((TCCR0 & 7) == 5, "делитель 1024 (CS02, CS00)");
  int prev = pin_out(LED), changes = 0; double t[2] = {-1, -1};
  for(int i = 0; i < 10000; i++){                 /* 1 с шагами по 0,1 мс */
    run_ms(0.1);
    int v = pin_out(LED);
    if(v != prev){ if(changes < 2) t[changes] = sim_time_ms(); changes++; prev = v; }
  }
  CHECK(changes == 30, "за 1 с 30 переключений (меандр 15,26 Гц)");
  CHECK(t[0] > 32.7 && t[0] < 32.9 && t[1] > 65.5 && t[1] < 65.7, "переключения каждые 32,768 мс");
  CHECK(TIMSK == 0, "прерывания не нужны: TIMSK=0");
  return sim_done("07/oc0");
}
