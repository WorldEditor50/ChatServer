#include "server.h"
/* register interface */
ListInterface* g_pstIfUserList = &g_stIfList;
ThreadPoolInterface* g_pstIfTPool = &g_stIfTPool;
MessageInterface* g_pstIfMessage = &g_stIfMessage;

RequestMethod g_pfRequestMethod[SERVER_REQUEST_METHOD_NUM] = {
    Server_Transfer,
    Server_Broadcast,
    Server_GetAllUser,
    Server_Logout,
    Server_KickOut,
    Server_BanTalk,
    Server_ApplyForAdministrator,
    Server_Reflect
};
char g_bTimeOutFlag = 0;

void Server_SetTimeOut(int sigNum)
{
    g_bTimeOutFlag = 1;
    return;
}

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
            break;
        }
        pstNode = pstNode->pstNext;
    }
    return pstUser; 
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

/* memory */
Server* Server_New()
{
    /* init memory */
    Server* pstServer = NULL;
    do {
        pstServer = (Server*)malloc(sizeof(Server));
        if (pstServer == NULL) {
            SERVER_LOG("fail to create server");
            break;
        }
        /* create user list */
        pstServer->pstUserList = g_pstIfUserList->pfNew();
        if (pstServer->pstUserList == NULL) {
            SERVER_LOG("fail to create user list");
            break;
        }
        /* register object: user */
        g_pstIfUserList->pfRegisterObject(pstServer->pstUserList,
                User_NewAdapter,
                User_DeleteAdapter,
                User_CompareAdapter,
                User_WriteAdapter,
                User_ReadAdapter);
        /* create create pool */
        pstServer->pstTPool = g_pstIfTPool->pfNew(4, 16, 100);
        if (pstServer->pstTPool == NULL) {
            SERVER_LOG("fail to create thread pool");
            break;
        }
        pstServer->fd = -1;
        pstServer->pstLock = &(pstServer->pstTPool->stLock);
        pstServer->pfMessageFilter = NULL;
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
    if (g_pstIfUserList != NULL && 
            pstServer->pstUserList != NULL) {
        g_pstIfUserList->pfDelete(pstServer->pstUserList);
    }
    if (g_pstIfTPool != NULL &&
            pstServer->pstTPool != NULL) {
        g_pstIfTPool->pfDelete(pstServer->pstTPool);
    }
    return SERVER_OK;
}

/* state */
int Server_Init(Server* pstServer)
{

    return SERVER_OK;
}

int Server_Shutdown(Server* pstServer)
{
    /* shutdown thread pool */
    g_pstIfTPool->pfShutdown(pstServer->pstTPool);
    /* close all fd */
    User* pstUser = NULL;
    Node *pstNode = pstServer->pstUserList->pstHead;
    while (pstNode != NULL) {
        pstUser = (User*)pstNode->pvInstance;
        close(pstUser->fd);
        pstNode = pstNode->pstNext;
    }
    return SERVER_OK;
}

/* user */
int Server_AddUser(List* pstUserList, int fd, char* pcIpAddr, int port)
{
    if (pstUserList == NULL || pcIpAddr == NULL) {
        Log_Write(SERVER_LOGFILE, "fail to create user");
        return SERVER_NULL;
    }
    User* pstUser = NULL;
    pstUser = (User*)g_pstIfUserList->pfNew();
    if (pstUser == NULL) {
        Log_Write(SERVER_LOGFILE, "fail to create user");
        return SERVER_MEM_ERR;
    }
    strcpy(pstUser->acIpAddr, pcIpAddr);
    pstUser->port = port;
    pstUser->fd = fd;
    pstUser->state = USER_ONLINE;
    pstUser->stat = USER_NONADMIN;
    int ret = g_pstIfUserList->pfPushBack(pstUserList, (void*)pstUser);
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
User* Server_SearchUser(List* pstUserList, char* pcIpAddr, int port)
{   
    if (pstUserList == NULL || pcIpAddr == NULL) {
        return NULL;
    }
    User stUserTmp;
    User* pstUser = NULL;
    strcpy(stUserTmp.acIpAddr, pcIpAddr);
    stUserTmp.port = port;
    if (g_pstIfUserList == NULL) {
        pstUser = (User*)g_pstIfUserList->pfSearch(pstUserList, (void*)&stUserTmp);
    }
    return pstUser;
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
    if (pstServer->pstLock == NULL) {
        return SERVER_NULL;
    }
    if (g_pstIfUserList == NULL) {
        return SERVER_NULL;
    }
    if (g_pstIfTPool == NULL) {
        return SERVER_NULL;
    }
    if (g_pstIfMessage == NULL) {
        return SERVER_NULL;
    }
    int ret;
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
        pthread_mutex_lock(pstServer->pstLock);
        /* get peer info */
        ret = getpeername(reqFd, (struct sockaddr*)&stPeerAddr, &addrLen);
        if (ret < 0) {
            Log_Write(SERVER_LOGFILE, "getpeername error");
            pthread_mutex_unlock(pstServer->pstLock);
            continue;
        }
        memset(acPeerIpAddr, 0, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &stPeerAddr.sin_addr, acPeerIpAddr, INET_ADDRSTRLEN);
        peerPort = ntohs(stPeerAddr.sin_port);
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
        /* add task to receive message */
        (void)g_pstIfTPool->pfAddTask(pstServer->pstTPool, Server_Recv, (void*)pstServer);
        pthread_mutex_unlock(pstServer->pstLock);
    }
    return SERVER_OK;
}

void* Server_Send(void* pvArg)
{

    return NULL;
}

void* Server_Recv(void* pvArg)
{
    Server* pstServer = (Server*)pvArg;
    int type = MESSAGE_REFLECT;
    int ret;
    char acMessage[MESSAGE_MAX_LEN];
    /* timer */
    signal(SIGALRM, Server_SetTimeOut);
    struct itimerval stNewValue;
    struct itimerval stOldValue;
    stNewValue.it_value.tv_sec = 0;
    stNewValue.it_value.tv_usec = 1;
    stNewValue.it_interval.tv_sec = 0;
    stNewValue.it_interval.tv_usec = 200000;
    setitimer(ITIMER_REAL, &stNewValue, &stOldValue);
    while (1) {
        /* recv message */
        recv(pstServer->fd, (void*)acMessage, MESSAGE_MAX_LEN, 0);
        pthread_mutex_lock(pstServer->pstLock);
        /* message filter */
        if (pstServer->pfMessageFilter != NULL) {
            ret = pstServer->pfMessageFilter(acMessage);
            if (ret != SERVER_OK) {
                break;
            }
        }
        /* get request type */
        type = g_pstIfMessage->pfGetType(acMessage);
        if (type < 0) {
            type = MESSAGE_REFLECT; 
        } 
        type = type % SERVER_REQUEST_METHOD_NUM;
        /* response */
        ret = g_pfRequestMethod[type](pstServer, acMessage);
        if (ret < 0) {
            pthread_mutex_unlock(pstServer->pstLock);
            break;
        }
        /* time out */
        if (g_bTimeOutFlag == 1) {
            pthread_mutex_unlock(pstServer->pstLock);
            break;
        }
        pthread_mutex_unlock(pstServer->pstLock);
    }
    return NULL;
}

User* Server_SearchUserByMessage(Server* pstServer, char* pcMessage)
{
    int ret;
    char acName[USER_NAME_LEN];
    /* get user name */
    ret = g_pstIfMessage->pfGetSrcName(pcMessage, acName);
    if (ret != SERVER_OK) {
        return NULL;
    }
    if (strlen(acName) > USER_NAME_LEN) {
        return NULL;
    }
    /* search user */
    return Server_SearchUserByName(pstServer->pstUserList, acName);
}

/* request method */
int Server_Transfer(Server* pstServer, char* pcMessage)
{
    int ret;
    User* pstUser = NULL;
    char acName[USER_NAME_LEN];
    /* parse dst user */
    ret = g_pstIfMessage->pfGetDstName(pcMessage, acName);
    if (ret != SERVER_OK) {
        return ret;
    }
    if (strlen(acName) > USER_NAME_LEN) {
        return SERVER_NO_USER;
    }
    /* search user */
    pstUser = Server_SearchUserByName(pstServer->pstUserList, acName);
    if (pstUser == NULL) {
        return SERVER_NOT_FOUND;
    }
    /* check user state */
    if (pstUser->state != USER_ONLINE) {
        return SERVER_INVALID_USERSTATE;
    }
    /* send message */
    send(pstUser->fd, pcMessage, strlen(pcMessage), 0);
    return SERVER_OK;
}

int Server_Broadcast(Server* pstServer, char* pcMessage)
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

int Server_GetAllUser(Server* pstServer, char* pcMessage)
{

    return SERVER_OK;
}

int Server_Logout(Server* pstServer, char* pcMessage)
{
    User* pstUser = NULL;
    char acOK[32] = "succeed to logout";
    /* search user */
    pstUser = Server_SearchUserByMessage(pstServer, pcMessage);
    if (pstUser == NULL) {
        return SERVER_NOT_FOUND;
    }
    /* set logout */
    pstUser->state = USER_LOGOUT;
    close(pstUser->fd);
    /* responese */
    send(pstUser->fd, acOK, strlen(acOK), 0);
    return -1;
}

int Server_KickOut(Server* pstServer, char* pcMessage)
{
    User* pstUser = NULL;
    char acOK[32] = "you have been kicked out";
    /* search user */
    pstUser = Server_SearchUserByMessage(pstServer, pcMessage);
    if (pstUser == NULL) {
        return SERVER_NOT_FOUND;
    }
    /* set kickout */
    pstUser->state = USER_KICKOUT;
    close(pstUser->fd);
    /* responese */
    send(pstUser->fd, acOK, strlen(acOK), 0);

    return SERVER_OK;
}

int Server_BanTalk(Server* pstServer, char* pcMessage)
{

    User* pstUser = NULL;
    char acOK[32] = "you have been banned to talk";
    /* search user */
    pstUser = Server_SearchUserByMessage(pstServer, pcMessage);
    if (pstUser == NULL) {
        return SERVER_NOT_FOUND;
    }
    /* set bantalk */
    pstUser->state = USER_BANTALK;
    /* responese */
    send(pstUser->fd, acOK, strlen(acOK), 0);
    return SERVER_OK;
}

int Server_ApplyForAdministrator(Server* pstServer, char* pcMessage)
{
    User* pstUser = NULL;
    char acOK[32] = "succeed to be administator";
    /* search user */
    pstUser = Server_SearchUserByMessage(pstServer, pcMessage);
    if (pstUser == NULL) {
        return SERVER_NOT_FOUND;
    }
    /* set admin */
    pstUser->stat = USER_ADMIN;
    /* responese */
    send(pstUser->fd, acOK, strlen(acOK), 0);
    return SERVER_OK;
}

int Server_Reflect(Server* pstServer, char* pcMessage)
{
    int ret;
    User* pstUser = NULL;
    char acName[USER_NAME_LEN];
    /* get user name */
    ret = g_pstIfMessage->pfGetSrcName(pcMessage, acName);
    if (ret != SERVER_OK) {
        return SERVER_MESSAGE_ERR;
    }
    if (strlen(acName) > USER_NAME_LEN) {
        return SERVER_MESSAGE_ERR;
    }
    /* search user */
    pstUser = Server_SearchUserByName(pstServer->pstUserList, acName);
    if (pstUser == NULL) {
        return SERVER_NO_USER;
    }
    /* check user state */
    if (pstUser->state != USER_ONLINE) {
        return SERVER_USER_OFFLINE; 
    }
    /* send message */
    send(pstUser->fd, pcMessage, strlen(pcMessage), 0);
    return SERVER_OK;
}

int Server_Register(Server* pstServer, char* pcMessage)
{
    int ret;
    int port;
    User* pstUser = NULL;
    char acIpAddr[INET_ADDRSTRLEN];
    char acName[USER_NAME_LEN];
    char acOK[32] = "succeed to register";
    /* get user ip */
    ret = g_pstIfMessage->pfGetIpAddr(pcMessage, acIpAddr);
    if (ret != SERVER_OK) {
        return SERVER_MESSAGE_ERR;
    }
    /* get user port */
    port = g_pstIfMessage->pfGetPort(pcMessage);
    if (port < 8000) {
        return SERVER_INVALID_PORT;
    }
    /* get user name */
    ret = g_pstIfMessage->pfGetSrcName(pcMessage, acName);
    if (ret != SERVER_OK) {
        return SERVER_MESSAGE_ERR;
    }
    if (strlen(acName) > USER_NAME_LEN) {
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
