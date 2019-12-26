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
#include <fcntl.h>
#include "../container/list/list.h"
#include "../threadpool/threadpool.h"
#include "../message/message.h"
#include "../user/user.h"
#include "../log/log.h"

#define SERVER_LOGFILE "./server_log"
#define SERVER_SHUTDOWN   0
#define SERVER_RUNNING    1
#define SERVER_LOG(message) do { \
    LOG_WRITE(SERVER_LOGFILE, message); \
} while (0)

#define REQUEST_METHOD_NUM 10
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
    SERVER_INVALID_USER
} SERVER_ENUM;

ListInterface* g_pstIfList;
ThreadPoolInterface* g_pstIfTPool;
MessageInterface* g_pstIfMessage;

typedef struct Server Server;
typedef struct Connect Connect;
typedef struct Request Request;

struct Server {
    int fd;
    List* pstUserList;
    List* pstReqMemPool;
    ThreadPool* pstTPool;
    ThreadPool* pstConnectPool;
    pthread_mutex_t stLock;
    int (*pfMessageFilter)(char* pcMessage);
    int state;
};
struct Connect {
    Server* pstServer;
    int reqFd;
};
struct Request {
    Server* pstServer;
    int type;
    char acMessage[MESSAGE_MAX_LEN];
};
typedef int (*RequestMethod)(Server* pstServer, char* pcMessage);
RequestMethod g_pfRequestMethod[REQUEST_METHOD_NUM];

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
int Request_DeleteAdapter(void* pvInstance);
Request* Request_New(List* pstReqMemPool);
void* Request_Handler(void* pvArg);
int Request_Transfer(Server* pstServer, char* pcMessage);
int Request_Broadcast(Server* pstServer, char* pcMessage);
int Request_GetAllUser(Server* pstServer, char* pcMessage);
int Request_Logout(Server* pstServer, char* pcMessage);
int Request_KickOut(Server* pstServer, char* pcMessage);
int Request_BanTalk(Server* pstServer, char* pcMessage);
int Request_ApplyForAdministrator(Server* pstServer, char* pcMessage);
int Request_Reflect(Server* pstServer, char* pcMessage);
int Request_Register(Server* pstServer, char* pcMessage);
int Request_Shutdown(Server* pstServer, char* pcMessage);
#endif // SERVER_H
