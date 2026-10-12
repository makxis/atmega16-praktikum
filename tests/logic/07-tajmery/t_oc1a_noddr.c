/* Мутант oc1a.c без DDRD (tests/mutants/07-oc1a-noddr.json): PD5 остался входом.
   Таймер считает и переключает OC1A внутри, но на гнездо сигнал не выходит. */
#include "sim.h"
#include "07-tajmery/oc1a__noddr.c"
#define LED PIN('D', 5)
int main(void){
  sim_start();
  int prev = pin_level(LED), changes = 0;
  for(int ms = 0; ms < 2000; ms++){ run_ms(1); int v = pin_level(LED); if(v != prev){ changes++; prev = v; } }
  CHECK(pin_out(LED) == -1, "мутант: PD5 - вход");
  CHECK(changes == 0, "мутант ловится: без DDRD на PD5 нет сигнала таймера");
  return sim_done("07/oc1a__noddr (мутант)");
}
