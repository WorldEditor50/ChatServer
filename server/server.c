#include "server.h"
/* register interface */
ListInterface* g_pstIfList = &g_stIfList;
ThreadPoolInterface* g_pstIfTPool = &g_stIfTPool;
MessageInterface* g_pstIfMessage = &g_stIfMessage;

RequestMethod g_pfRequestMethod[REQUEST_METHOD_NUM] = {
    Request_Transfer,
    Request_Broadcast,
    Request_GetAllUser,
    Request_Logout,
    Request_KickOut,
    Request_BanTalk,
    Request_ApplyForAdministrator,
    Request_Reflect,
    Request_Register,
    Request_Shutdown
};

User* Server_SearchUserByName(List* pstList, char* pcName)
{
    if (pstList == NULL || pstList->pstHead == NULL || pcName == NULL) {
        return NULL;
    }
    User* pstUser = NULL;
    Node *pstNode = pstList->pstHead;
    while (pstNode != NULL) {
        pstUser = (User*)pstNode->pvInstance;
        if (strcmp(pstUser->acName, pcName) == 0) {
            return pstUser;
        }
        pstNode = pstNode->pstNext;
    }
    return NULL; 
}

int Server_SearchUserByFd(List* pstList, int fd)
{
    if (pstList == NULL || pstList->pstHead == NULL) {
        return SERVER_NULL;
    }
    User* pstUser = NULL;
    Node *pstNode = pstList->pstHead;
    while (pstNode != NULL) {
        pstUser = (User*)pstNode->pvInstance;
        if (pstUser->fd == fd) {
            return SERVER_OK;
        }
        pstNode = pstNode->pstNext;
    }
    return SERVER_NOT_FOUND;
}

User* Server_SearchUser(List* pstUserList, char* pcIpAddr, int port)
{   
    if (pstUserList == NULL || pcIpAddr == NULL) {
        return NULL;
    }
    User* pstUser = NULL;
    Node *pstNode = pstUserList->pstHead;
    while (pstNode != NULL) {
        pstUser = (User*)pstNode->pvInstance;
        if (pstUser->port == port && strcmp(pstUser->acIpAddr, pcIpAddr) == 0) {
            return pstUser;
        }
        pstNode = pstNode->pstNext;
    }
    return NULL;
}

User* Server_SearchSrcUser(Server* pstServer, char* pcMessage)
{
    int ret;
    char acName[USER_NAMELEN];
    /* get user name */
    ret = g_pstIfMessage->pfGetSrcName(pcMessage, acName);
    if (ret != SERVER_OK) {
        return NULL;
    }
    if (strlen(acName) > USER_NAMELEN) {
        return NULL;
    }
    /* search user */
    return Server_SearchUserByName(pstServer->pstUserList, acName);
}

User* Server_SearchDstUser(Server* pstServer, char* pcMessage)
{
    int ret;
    char acName[USER_NAMELEN];
    /* get user name */
    ret = g_pstIfMessage->pfGetDstName(pcMessage, acName);
    if (ret != SERVER_OK) {
        return NULL;
    }
    if (strlen(acName) > USER_NAMELEN) {
        return NULL;
    }
    /* search user */
    return Server_SearchUserByName(pstServer->pstUserList, acName);
}

/* memory */
Server* Server_New()
{
    /* check interface */
    if (g_pstIfList == NULL || g_pstIfTPool == NULL ||
            g_pstIfMessage == NULL) {
        return NULL;
    }
    /* init memory */
    Server* pstServer = NULL;
    do {
        pstServer = (Server*)malloc(sizeof(Server));
        if (pstServer == NULL) {
            SERVER_LOG("fail to create server");
            break;
        }
        /* create user list */
        pstServer->pstUserList = g_pstIfList->pfNew();
        if (pstServer->pstUserList == NULL) {
            SERVER_LOG("fail to create user list");
            break;
        }
        /* register object: user */
        g_pstIfList->pfRegisterObject(pstServer->pstUserList,
                User_NewAdapter,
                User_DeleteAdapter,
                User_CompareAdapter,
                User_WriteAdapter,
                User_ReadAdapter);
        /* create request mem pool */
        pstServer->pstReqMemPool = g_pstIfList->pfNew();
        if (pstServer->pstReqMemPool == NULL) {
            SERVER_LOG("fail to request create mem pool");
            break;
        }
        /* register object: request */
        g_pstIfList->pfRegisterObject(pstServer->pstReqMemPool,
                NULL,
                Request_DeleteAdapter,
                NULL,
                NULL,
                NULL);
        /* create lock for server */
        int ret = pthread_mutex_init(&pstServer->stLock, NULL);
        if (ret < 0) {
            break;
        }
        /* create thread pool to handle task */
        pstServer->pstTPool = g_pstIfTPool->pfNew(4, 16, 100);
        if (pstServer->pstTPool == NULL) {
            SERVER_LOG("fail to create thread pool");
            break;
        }
        /* create thread to recieve message */
        ret = pthread_create(&(pstServer->recvTid), NULL, Server_RecvAll, (void*)pstServer);
        if (ret < 0) {
            break;
        }
        pstServer->fd = -1;
        pstServer->pfMessageFilter = NULL;
        pstServer->state = SERVER_RUNNING;
        /* empty log */
        FILE* pstFile = NULL;
        pstFile = fopen(SERVER_LOGFILE, "w");
        fclose(pstFile);
        return pstServer;
    } while (0);
    Server_Delete(pstServer);
    return NULL;
}

int Server_Delete(Server* pstServer)
{
    if (pstServer == NULL) {
        return SERVER_NULL;
    }
    /* delete user list */
    if (g_pstIfList != NULL && 
            pstServer->pstUserList != NULL) {
        g_pstIfList->pfDelete(pstServer->pstUserList);
    }
    printf("%s\n", "delete user list done");
    /* delete memory pool */
    if (g_pstIfList != NULL && 
            pstServer->pstReqMemPool != NULL) {
        g_pstIfList->pfDelete(pstServer->pstReqMemPool);
    }
    printf("%s\n", "delete mem pool done");
    /* delete thread pool */
    if (g_pstIfTPool != NULL && 
            pstServer->pstTPool != NULL) {
        g_pstIfTPool->pfDelete(pstServer->pstTPool);
    }
    printf("%s\n", "delete threadpool done");
    /* destroy lock */
    pthread_mutex_lock(&pstServer->stLock);
    pthread_mutex_destroy(&pstServer->stLock);
    printf("%s\n", "delete mutex lock done");
    /* free all */
    free(pstServer);
    printf("%s\n", "complete");
    return SERVER_OK;
}

/* state */
int Server_Init(Server* pstServer)
{

    return SERVER_OK;
}

int Server_Shutdown(Server* pstServer)
{
    /* close all fd */
    pthread_mutex_lock(&pstServer->stLock);
    User* pstUser = NULL;
    Node *pstNode = pstServer->pstUserList->pstHead;
    while (pstNode != NULL) {
        pstUser = (User*)pstNode->pvInstance;
        close(pstUser->fd);
        pstNode = pstNode->pstNext;
    }
    pthread_mutex_unlock(&pstServer->stLock);
    printf("%s\n", "calling pool shutdown");
    close(pstServer->fd);
    /* shutdown thread pool */
    g_pstIfTPool->pfShutdown(pstServer->pstTPool);
    return SERVER_OK;
}

void* Server_AsyncClose(void* pvArg)
{
    Server* pstServer = (Server*)pvArg;
    Server_Shutdown(pstServer);
    Server_Delete(pstServer);
    printf("%s\n", "delete completly");
    pthread_detach(pthread_self());
    return NULL;
}

/* user */
int Server_AddUser(List* pstUserList, int fd, char* pcIpAddr, int port)
{
    if (pstUserList == NULL || pcIpAddr == NULL) {
        Log_Write(SERVER_LOGFILE, "fail to create user");
        return SERVER_NULL;
    }
    User* pstUser = NULL;
    pstUser = (User*)g_pstIfList->pfNew();
    if (pstUser == NULL) {
        Log_Write(SERVER_LOGFILE, "fail to create user");
        return SERVER_MEM_ERR;
    }
    strcpy(pstUser->acIpAddr, pcIpAddr);
    pstUser->port = port;
    pstUser->fd = fd;
    pstUser->state = USER_ONLINE;
    pstUser->stat = USER_NONADMIN;
    int ret = g_pstIfList->pfPushBack(pstUserList, (void*)pstUser);
    if (ret != SERVER_OK) {
        Log_Write(SERVER_LOGFILE, "fail to add user");
        return SERVER_ERR;
    }
    return SERVER_OK;
}

int Server_DetectUser(Server* pstServer)
{

    return SERVER_OK;
}

/* tcp connection */
int Server_TcpListen(Server* pstServer, int port)
{
    if (pstServer == NULL) {
        return SERVER_NULL;
    }
    if (port < 0) {
        return SERVER_INVALID_PORT;
    }
    int ret;
    /* create socket */
    pstServer->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (pstServer->fd < 0) {
        SERVER_LOG("create socket fail");
        return SERVER_SOCKET_ERR; 
    }
    /* bind */
    struct sockaddr_in stAddr;
    stAddr.sin_family = AF_INET;
    stAddr.sin_port = htons(port);
    stAddr.sin_addr.s_addr = INADDR_ANY;
    ret = bind(pstServer->fd, (struct sockaddr*)&stAddr, sizeof(struct sockaddr_in));
    if (ret < 0) {
        SERVER_LOG("bind error");
        return ret;
    }
    /* listen */
    ret = listen(pstServer->fd, 127);
    if (ret < 0) {
        SERVER_LOG("listen error");
        return ret;
    }
    printf("%s\n", "server is listening");
    return SERVER_OK;
}

int Server_Run(Server* pstServer)
{
    if (pstServer == NULL) {
        return SERVER_NULL;
    }
    if (pstServer->pstUserList == NULL) {
        return SERVER_NULL;
    }
    if (pstServer->pstTPool == NULL) {
        return SERVER_NULL;
    }
    if (g_pstIfList == NULL) {
        return SERVER_NULL;
    }
    if (g_pstIfTPool == NULL) {
        return SERVER_NULL;
    }
    if (g_pstIfMessage == NULL) {
        return SERVER_NULL;
    }
    int ret;
    int flags;
    socklen_t addrLen = 0;
    struct sockaddr_in stAddr;
    struct sockaddr_in stPeerAddr;
    char acPeerIpAddr[INET_ADDRSTRLEN];
    int peerPort = 0;
    while (1) {
        /* accept */
        int reqFd = accept(pstServer->fd, (struct sockaddr*)&stAddr, &addrLen);
        if (reqFd < 0) {
            Log_Write(SERVER_LOGFILE, "accept error");
            continue;
        }
        pthread_mutex_lock(&pstServer->stLock);
        /* set socket nonblock */
        flags = fcntl(reqFd, F_GETFL, 0);
        fcntl(reqFd, F_SETFL, flags | O_NONBLOCK);
        /* get peer info */
        ret = getpeername(reqFd, (struct sockaddr*)&stPeerAddr, &addrLen);
        if (ret < 0) {
            Log_Write(SERVER_LOGFILE, "getpeername error");
            pthread_mutex_unlock(&pstServer->stLock);
            continue;
        }
        memset(acPeerIpAddr, 0, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &stPeerAddr.sin_addr, acPeerIpAddr, INET_ADDRSTRLEN);
        peerPort = ntohs(stPeerAddr.sin_port);
        printf("ip:%s:%d join in group\n", acPeerIpAddr, peerPort);
        /* search ip and port */
        User* pstUser = NULL;
        pstUser = Server_SearchUser(pstServer->pstUserList, acPeerIpAddr, peerPort);
        if (pstUser != NULL) {
            pstUser->state = USER_ONLINE;
            pstUser->fd = reqFd;
        } else {
            /* add user */
            ret = Server_AddUser(pstServer->pstUserList, reqFd, acPeerIpAddr, peerPort);
            if (ret != SERVER_OK) {
                Log_Write(SERVER_LOGFILE, "add user error");
            }
        }
        pthread_mutex_unlock(&pstServer->stLock);
    }
    return SERVER_OK;
}

void* Server_Send(void* pvArg)
{

    return NULL;
}

void* Server_RecvAll(void* pvArg)
{
    Server* pstServer = (Server*)pvArg;
    char acMessage[MESSAGE_MAX_LEN];
    int type = MESSAGE_REFLECT;
    ssize_t len;
    User* pstUser = NULL;
    Node* pstNode = NULL;
    while (1) {
        pthread_mutex_lock(&pstServer->stLock);
        if (pstServer->state == SERVER_SHUTDOWN) {
            pthread_mutex_unlock(&pstServer->stLock);
            break;
        }
        /* recv message */
        pstNode = pstServer->pstUserList->pstHead;
        while (pstNode != NULL) {
            pstUser = (User*)pstNode->pvInstance;
            memset(acMessage, 0, MESSAGE_MAX_LEN);
            len = recv(pstUser->fd, (void*)acMessage, MESSAGE_MAX_LEN, 0);
            if (len > 0) {
                printf("recv: %s\n", acMessage);
                /* get request type */
                type = g_pstIfMessage->pfGetType(acMessage);
                if (type < 0) {
                    type = MESSAGE_REFLECT; 
                } 
                /* add task to handle request */
                Node* pstNode = Request_New(pstServer->pstReqMemPool);
                if (pstNode != NULL) {
                    Request* pstReq = (Request*)pstNode->pvInstance;
                    pstReq->pstServer = pstServer;
                    pstReq->type = type % REQUEST_METHOD_NUM;
                    memset(pstReq->acMessage, 0, MESSAGE_MAX_LEN);
                    strcpy(pstReq->acMessage, acMessage);
                    g_pstIfTPool->pfAddTaskWithLock(pstServer->pstTPool, Request_Handler, (void*)pstNode);
                } else {
                    Log_Write(SERVER_LOGFILE, "add request error");
                }
            }
            pstNode = pstNode->pstNext;
        }
        pthread_mutex_unlock(&pstServer->stLock);
    }
    pthread_detach(pthread_self());
    return NULL;
}

/* request method */
void* Request_Handler(void* pvArg)
{
    Node* pstNode = (Node*)pvArg;
    Request* pstReq = (Request*)pstNode->pvInstance;
    Server* pstServer = pstReq->pstServer;
    /* message filter */
    if (pstServer->pfMessageFilter != NULL) {
        pstServer->pfMessageFilter(pstReq->acMessage);
    }
    /* response */
    pthread_mutex_lock(&pstServer->stLock);
    g_pfRequestMethod[pstReq->type](pstServer, pstReq->acMessage);
    /* recycle  */
    g_pstIfList->pfAddNodeToBack(pstServer->pstReqMemPool, pstNode);
    pthread_mutex_unlock(&pstServer->stLock);
    return NULL;
}

int Request_Transfer(Server* pstServer, char* pcMessage)
{
    User* pstUser = NULL;
    /* search dst user */
    pstUser = Server_SearchDstUser(pstServer, pcMessage);
    if (pstUser == NULL) {
        return SERVER_NOT_FOUND;
    }
    /* check user state */
    if (pstUser->state != USER_ONLINE) {
        return SERVER_INVALID_USER;
    }
    /* send message */
    send(pstUser->fd, pcMessage, strlen(pcMessage), 0);
    return SERVER_OK;
}

int Request_Broadcast(Server* pstServer, char* pcMessage)
{
    User* pstUser = NULL;
    Node *pstNode = pstServer->pstUserList->pstHead;
    while (pstNode != NULL) {
        pstUser = (User*)pstNode->pvInstance;
        send(pstUser->fd, pcMessage, strlen(pcMessage), 0);
        pstNode = pstNode->pstNext;
    }
    return SERVER_OK;
}

int Request_GetAllUser(Server* pstServer, char* pcMessage)
{
    User* pstUser = NULL;
    User* pstSrcUser = NULL;
    Node *pstNode = pstServer->pstUserList->pstHead;
    /* search src user */
    pstSrcUser = Server_SearchSrcUser(pstServer, pcMessage);
    if (pstSrcUser == NULL) {
        return SERVER_NOT_FOUND;
    }
    /* send user name */
    while (pstNode != NULL) {
        pstUser = (User*)pstNode->pvInstance;
        send(pstSrcUser->fd, pstUser->acName, strlen(pstUser->acName), 0);
        pstNode = pstNode->pstNext;
    }
    return SERVER_OK;
}

int Request_Logout(Server* pstServer, char* pcMessage)
{
    User* pstUser = NULL;
    char acOK[32] = "logout";
    /* search user */
    pstUser = Server_SearchSrcUser(pstServer, pcMessage);
    if (pstUser == NULL) {
        return SERVER_NOT_FOUND;
    }
    /* set logout */
    pstUser->state = USER_LOGOUT;
    /* responese */
    send(pstUser->fd, acOK, strlen(acOK), 0);
    close(pstUser->fd);
    return -1;
}

int Request_KickOut(Server* pstServer, char* pcMessage)
{
    User* pstSrcUser = NULL;
    User* pstDstUser = NULL;
    char acOK[32] = "you have been kicked out";
    /* search src user */
    pstSrcUser = Server_SearchSrcUser(pstServer, pcMessage);
    if (pstSrcUser == NULL) {
        return SERVER_NOT_FOUND;
    }
    /* check src user */
    if (pstSrcUser->stat != USER_ADMIN) {
        return SERVER_INVALID_USER;
    }
    /* search dst user */
    pstDstUser = Server_SearchDstUser(pstServer, pcMessage);
    if (pstDstUser == NULL) {
        return SERVER_NOT_FOUND;
    }
    /* set kickout */
    pstDstUser->state = USER_KICKOUT;
    /* responese */
    send(pstDstUser->fd, acOK, strlen(acOK), 0);
    close(pstDstUser->fd);
    return SERVER_OK;
}

int Request_BanTalk(Server* pstServer, char* pcMessage)
{
    User* pstSrcUser = NULL;
    User* pstDstUser = NULL;
    char acOK[32] = "you have been banned talking";
    /* search src user */
    pstSrcUser = Server_SearchSrcUser(pstServer, pcMessage);
    if (pstSrcUser == NULL) {
        return SERVER_NOT_FOUND;
    }
    /* check src user */
    if (pstSrcUser->stat != USER_ADMIN) {
        return SERVER_INVALID_USER;
    }
    /* search dst user */
    pstDstUser = Server_SearchDstUser(pstServer, pcMessage);
    if (pstDstUser == NULL) {
        return SERVER_NOT_FOUND;
    }
    /* set bantalk */
    pstDstUser->state = USER_BANTALK;
    /* responese */
    send(pstDstUser->fd, acOK, strlen(acOK), 0);
    return SERVER_OK;
}

int Request_ApplyForAdministrator(Server* pstServer, char* pcMessage)
{
    User* pstUser = NULL;
    char acOK[32] = "succeed to be an administator";
    /* search user */
    pstUser = Server_SearchSrcUser(pstServer, pcMessage);
    if (pstUser == NULL) {
        return SERVER_NOT_FOUND;
    }
    /* set admin */
    pstUser->stat = USER_ADMIN;
    /* responese */
    send(pstUser->fd, acOK, strlen(acOK), 0);
    return SERVER_OK;
}

int Request_Reflect(Server* pstServer, char* pcMessage)
{
    User* pstSrcUser = NULL;
    /* search src user */
    pstSrcUser = Server_SearchSrcUser(pstServer, pcMessage);
    if (pstSrcUser == NULL) {
        return SERVER_NOT_FOUND;
    }
    /* check user state */
    if (pstSrcUser->state != USER_ONLINE) {
        return SERVER_USER_OFFLINE; 
    }
    /* send message */
    send(pstSrcUser->fd, pcMessage, strlen(pcMessage), 0);
    return SERVER_OK;
}

int Request_Register(Server* pstServer, char* pcMessage)
{
    int ret;
    int port;
    User* pstUser = NULL;
    char acIpAddr[INET_ADDRSTRLEN];
    char acName[USER_NAMELEN];
    char acOK[32] = "succeed to register";
    /* get user ip */
    ret = g_pstIfMessage->pfGetIpAddr(pcMessage, acIpAddr);
    if (ret != SERVER_OK) {
        return SERVER_MESSAGE_ERR;
    }
    /* get user port */
    port = g_pstIfMessage->pfGetPort(pcMessage);
    if (port < 0) {
        return SERVER_INVALID_PORT;
    }
    /* get user name */
    ret = g_pstIfMessage->pfGetSrcName(pcMessage, acName);
    if (ret != SERVER_OK) {
        return SERVER_MESSAGE_ERR;
    }
    if (strlen(acName) > USER_NAMELEN) {
        return SERVER_MESSAGE_ERR;
    }
    /* search user */
    pstUser = Server_SearchUser(pstServer->pstUserList, acIpAddr, port);
    if (pstUser == NULL) {
        return SERVER_NO_USER;
    }
    /* set user name */
    strcpy(pstUser->acName, acName);
    /* registered */
    pstUser->isRegistered = 1;
    /* responese */
    send(pstUser->fd, acOK, strlen(acOK), 0);
    return SERVER_OK;
}

int Request_Shutdown(Server* pstServer, char* pcMessage)
{
    User* pstSrcUser = NULL;
    User* pstUser = NULL;
    char acOK[32] = "server is ready to shutdown";
    /* search src user */
    pstSrcUser = Server_SearchSrcUser(pstServer, pcMessage);
    if (pstSrcUser == NULL) {
        return SERVER_NOT_FOUND;
    }
    /* check src user */
    if (pstSrcUser->stat != USER_ADMIN) {
        return SERVER_INVALID_USER;
    }
    /* set the flag of shutdown  */
    pstServer->state = SERVER_SHUTDOWN;
    /* responese */
    send(pstSrcUser->fd, acOK, strlen(acOK), 0);
    /* close all connection */
    Node *pstNode = pstServer->pstUserList->pstHead;
    while (pstNode != NULL) {
        pstUser = (User*)pstNode->pvInstance;
        send(pstUser->fd, pcMessage, strlen(pcMessage), 0);
        pstNode = pstNode->pstNext;
    }
    /* async shutdown */
    pthread_t tid;
    pthread_create(&tid, NULL, Server_AsyncClose, (void*)pstServer);
    return -1;
}

int Request_DeleteAdapter(void* pvInstance)
{
    free(pvInstance);
    return SERVER_OK;
}

Node* Request_New(List* pstReqMemPool)
{
    if (pstReqMemPool == NULL) {
        return NULL;
    }
    Request* pstReq = NULL;
    Node* pstNode = NULL;
    pstNode = g_pstIfList->pfGetFront(pstReqMemPool); 
    if (pstNode == NULL) {
        pstReq = (Request*)malloc(sizeof(Request));
        if (pstReq == NULL) {
            return NULL;
        }
        pstNode = g_pstIfList->pfNewNode((void*)pstReq);
        if (pstNode == NULL) {
            return NULL;
        }
    }
    return pstNode;
}
