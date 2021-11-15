#include "threads/fixed_point.h"

int q_area = 1<<14;

fixed int_to_fixed(int n){
  return n * q_area;
}

int fixed_to_int(fixed a){
  if(a >= 0)
    return (a + q_area/2) / q_area;
  else
    return (a - q_area/2) / q_area;
}

fixed add_btw_fixed(fixed a, fixed b){
  return a + b;
}

fixed add_btw_diff(fixed a, int n){
  return a + n*q_area;
}
fixed sub_btw_fixed(fixed a, fixed b){
  return a - b;
}

fixed sub_btw_diff(fixed a, int n){
  return a - n*q_area;
}

fixed mult_btw_fixed(fixed a, fixed b){
  return ((uint64_t)a) * b / q_area;
}

fixed mult_btw_diff(fixed a, int n){
  return a * n;
}

fixed div_btw_fixed(fixed a, fixed b){
  return ((int64_t)a) * q_area / b;
}

fixed div_btw_diff(fixed a, int n){
  return a / n;
}
