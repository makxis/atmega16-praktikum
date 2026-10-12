/* Глава 7, задание 1 (oc1a_2hz.c): делитель 64, OCR1A=31249: OC1A переключается каждые 0,25 с (2 Гц). */
#include "sim.h"
#include "07-tajmery/oc1a_2hz.c"
#define LED PIN('D', 5)
int main(void){
  sim_start();
  CHECK(OCR1A == 31249, "OCR1A=31249");
  CHECK((TCCR1B & 7) == 3, "делитель 64 (CS11, CS10)");
  int prev = pin_out(LED), changes = 0; double t0 = -1;
  for(int ms = 0; ms < 3000; ms++){
    run_ms(1);
    int v = pin_out(LED);
    if(v != prev){ if(t0 < 0) t0 = sim_time_ms(); changes++; prev = v; }
  }
  CHECK(changes == 12, "за 3 с 12 переключений: мигание 2 Гц");
  CHECK(t0 > 249.9 && t0 < 251.1, "первое переключение через 0,25 с");
  return sim_done("07/oc1a_2hz");
}
