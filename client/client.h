#ifndef CLIENT_H
#define CLIENT_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include "../message/message.h"
#include "../log/log.h"

MessageInterface* g_pstIfMessage;
#define CLIENT_LOGFILE "./client_log"
#define CLIENT_LOG(message) do { \
    LOG_WRITE("./client_log", message); \
} while (0)
#define CLIENT_NAMELEN     64
#define CLIENT_CONTENTLEN  1024
#define CLIENT_RUNNING      0
#define CLIENT_SHUTDOWN     1
#define CLIENT_LOGOUT       2
typedef enum CLIENT_ENUM {
    CLIENT_OK,					
    CLIENT_ERR,				
    CLIENT_NULL,		
    CLIENT_MEM_ERR,		
    CLIENT_SIZE_ERR,
    CLIENT_NOT_FOUND,
    CLIENT_NO_FILE,
    CLIENT_SOCKET_ERR,
    CLIENT_CONNECT_ERR,
    CLIENT_THREAD_ERR
} CLIENT_ENUM;

typedef struct Client {
    int fd;
    int port;
    char acName[CLIENT_NAMELEN];
    int serverPort;
    char acServerIpAddr[INET_ADDRSTRLEN];
    char state;
    pthread_mutex_t stLock;
    pthread_t recvTid;
} Client;
/* client */
Client* Client_New();
int Client_Delete(Client* pstClient);
int Client_TcpConnect(Client* pstClient, char* pcIpAddr, int port);
int Client_TcpConnectByConfig(Client* pstClient);
int Client_Run(Client* pstClient);
int Client_Close(Client* pstClient);
/* configure */
int Client_ReadConfig(Client* pstClient, char* pcFileName);
/* message */
void Client_SendMessage(Client* pstClient, char* pcType);
void* Client_RecvMessage(void* pvArg);
#endif // CLIENT_H
