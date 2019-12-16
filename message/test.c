#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "message.h"

int main()
{
    char acWord[128] = "type:7,name:tom,ip:127.0.0.1,port:8090";
    char ip[32];
    Message_GetIpAddr(acWord, ip);
    printf("%s\n", ip);
	return 0;
}
