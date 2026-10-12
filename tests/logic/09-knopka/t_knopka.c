/* Глава 9, задание 1 (Knopka.c, программа преподавателя): подтяжка есть, пауза 0,6 с, автоповтор.
   Kod7 в файле намеренно не заполнен: тест вписывает коды, как это делают по заданию. */
#include "sim.h"
#include "09-knopka/Knopka.c"
#define BTN PIN('C', 1)
static const unsigned char KOD[10] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
/* Перед чтением индикатора - полный круг развёртки (4 вызова Timer0): после ручного T1() или правки
   переменных разряд, который горит в этот момент, иначе показал бы старый код. */
static int num(void){ T0(4); return disp_num(); }
int main(void){
  for(int d = 0; d < 10; d++) Kod7[d] = (char)KOD[d];
  sim_start();
  CHECK(pin_pullup(BTN), "Knopka.c: подтяжка на PC1 включена");
  T0(1000); CHECK(num()==0, "кнопку не трогаем 2 с: 0000");
  btn_press(BTN); CHECK(num()==1, "нажатие = 0001");
  T0(301); CHECK(num()==2, "держим: через 0,6 с ещё +1 (автоповтор)");
  btn_release(BTN); T0(400); CHECK(num()==2, "отпустили: 0002");
  t1 = 9; t2 = 9; t3 = 9; t4 = 9;
  btn_press(BTN); btn_release(BTN); T0(400); CHECK(num()==0, "9999 + 1 = 0000");
  return sim_done("09/Knopka");
}
