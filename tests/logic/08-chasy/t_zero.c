/* Глава 8, доработка 3 (zero.c): ведущий ноль погашен, " 5:07" вместо "05:07". */
#include "sim.h"
#include "08-chasy/zero.c"
/* Перед чтением индикатора - полный круг развёртки (4 вызова Timer0): после ручного T1() или правки
   переменных разряд, который горит в этот момент, иначе показал бы старый код. */
static int num(void){ T0(4); return disp_num(); }
int main(void){
  sim_start();
  T0(4); CHECK(disp_digit(0)==-1 && disp_digit(1)==0 && disp_digit(2)==0 && disp_digit(3)==0, "старт: _0:00");
  T1(5*60+7);
  T0(4); CHECK(disp_digit(0)==-1 && disp_digit(1)==5 && disp_digit(2)==0 && disp_digit(3)==7, "05:07 показано как _5:07");
  T1(10*60);
  T0(4); CHECK(disp_digit(0)==1 && num()==1507, "15:07: единица в десятках минут горит");
  T1(45*60);
  T0(4); CHECK(disp_digit(0)==-1 && num()==7, "60:07 -> _0:07, левый разряд снова пустой");
  return sim_done("08/zero");
}
