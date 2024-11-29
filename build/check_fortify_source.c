#include <stdio.h>
#ifndef _FORTIFY_SOURCE
#define _FORTIFY_SOURCE 0
#endif
int main(){printf("%d", _FORTIFY_SOURCE);return 0;}