/* Глава 8, задание 1 (Shasi.c, программа преподавателя): OCR1A не задан.
   Kod7 в файле намеренно не заполнен: тест вписывает коды, как это делают по заданию. */
#include "sim.h"
#include "08-chasy/Shasi.c"
static const unsigned char KOD[10] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
/* Перед чтением индикатора - полный круг развёртки (4 вызова Timer0): после ручного T1() или правки
   переменных разряд, который горит в этот момент, иначе показал бы старый код. */
static int num(void){ T0(4); return disp_num(); }
int main(void){
  for(int d = 0; d < 10; d++) Kod7[d] = (char)KOD[d];
  sim_start();
  CHECK(OCR1A==0, "в Shasi.c OCR1A не задан: после сброса 0");
  CHECK(num()==-1, "до первого прерывания Timer1 горит начальное содержимое VD (мусор)");
  T1(1); CHECK(num()==1, "логика счёта: 00:01");
  T1(59*60+58); CHECK(num()==5959, "59:59");
  T1(1); CHECK(num()==0, "59:59 -> 00:00");
  isr_calls_reset(); run_ms(1000);
  long n = isr_calls("TIMER1_COMPA");
  CHECK(n >= 31000 && n <= 31500, "OCR1A=0: около 31250 прерываний Timer1 в секунду (часы несутся)");
  long s = n % 3600;
  CHECK(num()==(int)((s / 60) * 100 + s % 60), "за 1 с часы убежали на столько секунд, сколько было прерываний");
  OCR1A = 31249; t1 = t2 = t3 = t4 = 0;        /* исправление из главы 8 */
  isr_calls_reset(); run_ms(3500);
  CHECK(isr_calls("TIMER1_COMPA")==3, "с OCR1A=31249: ровно 1 прерывание в секунду");
  CHECK(num()==3, "с исправлением через 3,5 с: 00:03");
  return sim_done("08/Shasi");
}
