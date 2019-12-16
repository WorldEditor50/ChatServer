#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "server.h"

int main()
{
    int ret;
    Server* pstServer = NULL;
    pstServer = Server_New();
    if (pstServer == NULL) {
        SERVER_LOG("fail to create server");
        return 0;
    }
    ret = Server_TcpListen(pstServer, 8022);
    if (ret != SERVER_OK) {
        SERVER_LOG("fail to listen");
    }
    Server_Run(pstServer);
    Server_Shutdown(pstServer);
    Server_Delete(pstServer);
	return 0;
}
