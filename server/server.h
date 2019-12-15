#ifndef SERVER_H
#define SERVER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "../container/list/list.h"
#include "../threadpool/threadpool.h"
#include "../message/message.h"
#include "../user/user.h"
#include "../log/log.h"

#define SERVER_LOGFILE "./server_log"
#define SERVER_IP_LEN   16
#define SERVER_NAME_LEN 64
#define SERVER_REQUEST_METHOD_NUM 8
#define SERVER_LOG(message) do { \
    LOG_WRITE(SERVER_LOGFILE, message); \
} while (0)

typedef enum SERVER_ENUM {
    SERVER_OK,
    SERVER_ERR,
    SERVER_NULL,
    SERVER_MEM_ERR,		
    SERVER_SIZE_ERR,
    SERVER_NOT_FOUND,
    SERVER_NO_FILE,
    SERVER_SOCKET_ERR,
    SERVER_INVALID_PORT,
    SERVER_MESSAGE_ERR,
    SERVER_NO_USER,
    SERVER_USER_OFFLINE,
    SERVER_INVALID_USERSTATE
} SERVER_ENUM;

ListInterface* g_pstIfUserList;
ThreadPoolInterface* g_pstIfTPool;
MessageInterface* g_pstIfMessage;

typedef struct Server {
    int fd;
    List* pstUserList;
    ThreadPool* pstTPool;
    pthread_mutex_t* pstLock;
    int (*pfMessageFilter)(char* pcMessage);
} Server;
typedef int (*RequestMethod)(Server* pstServer, char* pcMessage);
RequestMethod g_pfRequestMethod[SERVER_REQUEST_METHOD_NUM];
/* memory */
Server* Server_New();
int Server_Delete(Server* pstServer);
/* state */
int Server_Init(Server* pstServer);
int Server_Shutdown(Server* pstServer);
/* user */
int Server_AddUser(List* pstUserList, int fd, char* pcIpAddr, int port);
int Server_DetectUser(Server* pstServer);
User* Server_SearchUser(List* pstUserList, char* pcIpAddr, int port);
/* tcp connection */
int Server_TcpListen(Server* pstServer, int port);
int Server_SearchFD(Server* pstServer);
int Server_Run(Server* pstServer);
void* Server_Send(void* pvArg);
void* Server_Recv(void* pvArg);
/* message */
int Server_RegisterMessageFilter(Server pstServer, int (*pfMessageFilter)(char* pcMessage));
/* request method */
int Server_Transfer(Server* pstServer, char* pcMessage);
int Server_Broadcast(Server* pstServer, char* pcMessage);
int Server_GetAllUser(Server* pstServer, char* pcMessage);
int Server_Logout(Server* pstServer, char* pcMessage);
int Server_KickOut(Server* pstServer, char* pcMessage);
int Server_BanTalk(Server* pstServer, char* pcMessage);
int Server_ApplyForAdministrator(Server* pstServer, char* pcMessage);
int Server_Reflect(Server* pstServer, char* pcMessage);
#endif // SERVER_H
