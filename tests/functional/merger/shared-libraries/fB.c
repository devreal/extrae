#include <stdio.h>

extern void Extrae_user_function (int);

void fB(void)
{
	Extrae_user_function(1);
	printf ("fB\n");
	Extrae_user_function(0);
}