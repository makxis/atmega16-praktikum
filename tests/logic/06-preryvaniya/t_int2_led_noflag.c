/* Мутант int2_led.c без сброса флага (tests/mutants/06-int2_led-noflag.json): нет GIFR=(1<<INTF2).
   Смена ISC2 ставит INTF2, и после sei() INT2 срабатывает без всякого нажатия. */
#include "sim.h"
#include "06-preryvaniya/int2_led__noflag.c"
static long c0 = -1;
static int l0 = -1;
static void hook(int k, double ms){ (void)ms; if(k == 0){ c0 = isr_calls("INT2"); l0 = pin_out(PIN('B', 4)); } }
int main(void){
  sim_loop(2, hook);
  CHECK(c0 == 1 && l0 == 0, "мутант ловится: без сброса INTF2 светодиод переключился сам, без нажатия");
  return sim_done("06/int2_led__noflag (мутант)");
}
