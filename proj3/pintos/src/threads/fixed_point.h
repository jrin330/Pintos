#include <stdio.h>
#include <stdlib.h>

typedef int32_t fixed;

fixed int_to_fixed(int n);
int fixed_to_int(fixed a);
fixed add_btw_fixed(fixed a, fixed b);
fixed add_btw_diff(fixed a, int b);
fixed sub_btw_fixed(fixed a, fixed b);
fixed sub_btw_diff(fixed a, int b);
fixed mult_btw_fixed(fixed a, fixed b);
fixed mult_btw_diff(fixed a, int b);
fixed div_btw_fixed(fixed a, fixed b);
fixed div_btw_diff(fixed a, int b);
