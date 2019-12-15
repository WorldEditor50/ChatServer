#include "log.h"
int Log_Write(const char* pcFileName, const char* pcMessage)
{
	if (pcFileName == NULL || pcMessage == NULL) {
		return LOG_NULL;
	}
	FILE *pstFile = NULL;
	int iRet = LOG_OK;
	pstFile = fopen(pcFileName, "a");
	if (pstFile == NULL) {
		return LOG_NO_FILE;
	}
	iRet = fputs(pcMessage, pstFile);
	if (iRet < 0) {
		return LOG_ERR;
	}
	fclose(pstFile);
	return LOG_OK;
}
