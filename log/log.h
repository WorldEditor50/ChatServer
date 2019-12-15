#ifndef LOG_H
#define LOG_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define LOG_ERROR_NULL        "pointer is null"
#define LOG_ERROR_MEM     "fail to allocate memory"
#define LOG_ERROR_FOUND   "not found"
#define LOG_ERROR_FILE     "no such file"
enum LOG_ENUM {
	LOG_OK,
	LOG_NULL,
	LOG_NO_FILE,
	LOG_ERR
};
#define LOG_WRITE(pcFileName, pcMessage) do { \
	char acMessage[256]; \
	memset(acMessage, 0, 256); \
	sprintf(acMessage, \
            "file : %s  function : %s  line : %d  message : %s\n", \
			__FILE__, __FUNCTION__, __LINE__, pcMessage); \
	FILE *pstFile = NULL; \
	pstFile = fopen(pcFileName, "a"); \
	if (pstFile == NULL) { \
		break; \
	} \
	fputs(acMessage, pstFile); \
	fclose(pstFile); \
} while (0)
int Log_Write(const char* pcFileName, const char* pcMessage);
#endif // LOG_H
