#include "stl_str.h"

void stlFree(STP sVa1,STP sVa2)
{
	if (sVa1) stlFree(sVa1);
	if (sVa2) stlFree(sVa2);
}
void stlFree(STP sVa1,STP sVa2,STP sVa3,STP sVa4 /* = NULL */)
{
	stlFree(sVa1,sVa2);
	stlFree(sVa3,sVa4);
}
void stlFree(STP sVa1,STP sVa2,STP sVa3,STP sVa4,STP sVa5,STP sVa6 /* = NULL */)
{
	stlFree(sVa1,sVa2);
	stlFree(sVa3,sVa4);
	stlFree(sVa5,sVa6);
}