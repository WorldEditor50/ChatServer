#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"

int main()
{
    int ret;
    Client* pstClient = NULL;
    /* create client */
    pstClient = Client_New();
    if (pstClient == NULL) {
        return 0;
    }
    /* read config */
    ret = Client_ReadConfig(pstClient, "client_config");
    if (ret != CLIENT_OK) {
        goto EXIT;    
    }
    /* connect server */
    ret = Client_TcpConnectByConfig(pstClient);
    if (ret != CLIENT_OK) {
        goto EXIT;
    }
    /* run */
    Client_Run(pstClient);
    /* close */
    Client_Close(pstClient);
EXIT:
    Client_Delete(pstClient);
	return 0;
}
