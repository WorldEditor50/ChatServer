#include "log.h"

int main()
{
	int iRet = LOG_OK;
	iRet = Log_Write("test.txt", "test message\n");
	if (iRet != LOG_OK) {
		printf("%s\n", "error");
		return 0;
	}
	iRet = Log_Write("test.txt", "test message\n");
	if (iRet != LOG_OK) {
		printf("%s\n", "error");
		return 0;
	}
	LOG_WRITE("test2.txt", "hello log");
	return 0;
}
