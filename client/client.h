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
#include "../message/message.h"
#include "../log/log.h"

MessageInterface* g_pstIfMessage;
#define CLIENT_DEBUG 1
#if CLIENT_DEBUG
#define CLIENT_LOG(message) do { \
    LOG_WRITE("./client_log", message); \
} while (0)
#else
#define CLIENT_LOG(message)
#endif
#define CLIENT_NAME_LEN     64
#define CLIENT_CONTENT_LEN  1024
#define CLIENT_RUNNING      0
#define CLIENT_SHUTDOWN     1
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
    char acName[CLIENT_NAME_LEN];
    char state;
    pthread_mutex_t stLock;
    pthread_t recvTid;
} Client;
/* client */
Client* Client_New();
int Client_Delete(Client* pstClient);
int Client_TcpConnect(Client* pstClient, char* pcIpAddr, int port);
int Client_Run(Client* pstClient);
int Client_Close(Client* pstClient);
/* configure */
int Client_ReadConfig(Client* pstClient, char* pcFileName);
/* message */
void Client_SendMessage(Client* pstClient, char* pcType);
void* Client_RecvMessage(void* pvArg);
#endif // CLIENT_H
