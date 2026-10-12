/* Глава 7, задание 3 (mayak.c): Timer1, режим 14 (Fast PWM, вершина ICR1), инверсный ШИМ на OC1A (PD5):
   период 1 с, в начале каждого периода 0,1 с на выходе 0 (светодиод горит). */
#include "sim.h"
#include "07-tajmery/mayak.c"
#define LED PIN('D', 5)
int main(void){
  sim_start();
  int mode = (TCCR1A & 3) | ((TCCR1B >> 1) & 0x0C);
  CHECK(mode == 14, "режим 14: WGM13=1, WGM12=1, WGM11=1, WGM10=0");
  CHECK(ICR1 == 31249 && OCR1A == 3124, "ICR1=31249, OCR1A=3124");
  int lit_at[6];
  static const double at[6] = {50, 150, 950, 1050, 1150, 1990};
  double now = 0;
  for(int i = 0; i < 6; i++){ run_ms(at[i] - now); now = at[i]; lit_at[i] = (pin_out(LED) == 0); }
  CHECK(lit_at[0] && !lit_at[1] && !lit_at[2] && lit_at[3] && !lit_at[4] && !lit_at[5],
        "горит 0..0,1 с и 1,0..1,1 с, остальное время погашен");
  run_ms(10);                                     /* ровно 2 с от старта: начало периода */
  double d = pin_duty(LED, 3000);
  CHECK(d > 0.899 && d < 0.901, "за 3 периода на выходе 1 ровно 90 % времени (горит 10 %)");
  CHECK(TIMSK == 0, "прерывания не нужны: TIMSK=0");
  return sim_done("07/mayak");
}
