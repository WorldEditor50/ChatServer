#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "array.h"
ArrayInterface* g_pstArrayInterface = NULL;
int main()
{
    g_pstArrayInterface = &g_stArrayInterface;
    Array* pstArray = g_pstArrayInterface->pfNew(10);
    g_pstArrayInterface->pfDelete(pstArray);
	return 0;
}
