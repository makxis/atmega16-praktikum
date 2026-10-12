/* Мутант mayak.c с прямым ШИМ (tests/mutants/07-mayak-noninv.json): COM1A1:COM1A0 = 10.
   Светодиод горит от 0, поэтому он горит 0,9 с и гаснет на 0,1 с: маячок наоборот.
   Первый период пропускаем: при прямом ШИМ вывод становится 1 только при переходе TOP -> 0. */
#include "sim.h"
#include "07-tajmery/mayak__noninv.c"
int main(void){
  sim_start();
  run_ms(1000);
  double d = pin_duty(PIN('D', 5), 3000);
  CHECK(d > 0.099 && d < 0.101, "мутант ловится: на выходе 1 только 10 % времени, светодиод горит 90 %");
  return sim_done("07/mayak__noninv (мутант)");
}
