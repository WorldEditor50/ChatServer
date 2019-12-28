#include "client.h"
MessageInterface* g_pstIfMessage = &g_stIfMessage;
/* configure */
int Client_ReadConfig(Client* pstClient, char* pcFileName)
{
    if (pstClient == NULL || pcFileName == NULL) {
        return CLIENT_NULL;
    }
    char acBuffer[128] = {0};
    char* pcOffset = NULL;
    int len = 0;
    /* ClientName:Tom */
    FILE* pstFile = NULL;
    pstFile = fopen(pcFileName, "r");
    if (pstFile == NULL) {
        CLIENT_LOG("fail to read config file");
        return CLIENT_NO_FILE;
    }
    fgets(acBuffer, 128, pstFile);
    /* client name */
    pcOffset = strstr(acBuffer, "ClientName:");
    if (pcOffset == NULL) {
        fclose(pstFile);
        return CLIENT_NOT_FOUND;
    }
    strcpy(pstClient->acName, pcOffset + strlen("ClientName:"));
    len = strlen(pstClient->acName);
    pstClient->acName[len - 1] = '\0';
    printf("client name: %s\n", pstClient->acName);
    /* client port */
    memset(acBuffer, 0, 128);
    fgets(acBuffer, 128, pstFile);
    pcOffset = strstr(acBuffer, "ClientPort:");
    if (pcOffset == NULL) {
        fclose(pstFile);
        return CLIENT_NOT_FOUND;
    }
    pstClient->port = atoi(pcOffset + strlen("ClientPort:"));
    printf("client port: %d\n", pstClient->port);
    /* server ip */
    memset(acBuffer, 0, 128);
    fgets(acBuffer, 128, pstFile);
    pcOffset = strstr(acBuffer, "ServerIP:");
    if (pcOffset == NULL) {
        fclose(pstFile);
        return CLIENT_NOT_FOUND;
    }
    strcpy(pstClient->acServerIpAddr, pcOffset + strlen("ServerIP:"));
    len = strlen(pstClient->acServerIpAddr);
    pstClient->acServerIpAddr[len - 1] = '\0';
    printf("server ip: %s\n", pstClient->acServerIpAddr);
    /* server port */
    memset(acBuffer, 0, 128);
    fgets(acBuffer, 128, pstFile);
    pcOffset = strstr(acBuffer, "ServerPort:");
    if (pcOffset == NULL) {
        fclose(pstFile);
        return CLIENT_NOT_FOUND;
    }
    pstClient->serverPort = atoi(pcOffset + strlen("ServerPort:"));
    printf("server port: %d\n", pstClient->serverPort);
    fclose(pstFile);
    /* client can be run */
    pstClient->state = CLIENT_RUNNING;
    return CLIENT_OK;
}

/* client */
Client* Client_New()
{
    Client* pstClient = NULL;
    pstClient = (Client*)malloc(sizeof(Client));
    if (pstClient == NULL) {
        CLIENT_LOG("fail to allocte client"); 
        return NULL;
    }
    pstClient->fd = -1;
    pstClient->state = CLIENT_SHUTDOWN;
    /* mutex lock */
    pthread_mutex_init(&pstClient->stLock, NULL);
    /* empty log */
    FILE* pstFile = NULL;
    pstFile = fopen(CLIENT_LOGFILE, "w");
    fclose(pstFile);
    return pstClient;
}

int Client_Delete(Client* pstClient)
{
    if (pstClient == NULL) {
        return CLIENT_NULL;
    }
    pthread_mutex_lock(&pstClient->stLock);
    pthread_mutex_destroy(&pstClient->stLock);
    free(pstClient);
    return CLIENT_OK;
}

int Client_TcpConnect(Client* pstClient, char* pcIpAddr, int port)
{
    if (pstClient == NULL || pcIpAddr == NULL) {
        return CLIENT_NULL;
    }
    int ret;
    /* create socket */
    pstClient->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (pstClient->fd < 0) {
        CLIENT_LOG("fail to create socket");
        return CLIENT_SOCKET_ERR;
    }
    /* bind */
    struct sockaddr_in stAddr;
    stAddr.sin_family = AF_INET;
    stAddr.sin_port = htons(pstClient->port);
    stAddr.sin_addr.s_addr = INADDR_ANY;
    ret = bind(pstClient->fd, (struct sockaddr*)&stAddr, sizeof(stAddr));
    if (ret < 0) {
        CLIENT_LOG("fail to bind");
        close(pstClient->fd);
        return CLIENT_SOCKET_ERR;
    }
    /* connect */
    struct sockaddr_in stServerAddr;
    stServerAddr.sin_family = AF_INET;
    stServerAddr.sin_port = htons(port);
    stServerAddr.sin_addr.s_addr = inet_addr(pcIpAddr);
    ret = connect(pstClient->fd, (struct sockaddr*)&stServerAddr, sizeof(stServerAddr));
    if (ret < 0) {
        CLIENT_LOG("fail to connect server");
        close(pstClient->fd);
        return CLIENT_CONNECT_ERR;
    }
    return CLIENT_OK;
}

int Client_TcpConnectByConfig(Client* pstClient)
{
    return Client_TcpConnect(pstClient, pstClient->acServerIpAddr, pstClient->serverPort);
}

void Client_ShowUsage()
{
    printf("usage :\n");
    printf("TRANSFER -- send message to other user\n");
    printf("BROADCAST -- send message to all user\n");
    printf("ADMIN -- apply for administor\n");
    printf("SHUTDOWN -- shutdown server\n");
    printf("BANTALK -- ban talking\n");
    printf("KICKOUT -- kick out\n");
    printf("LOGOUT -- logout\n");
    printf("exit -- logout and leave\n");
    printf("usage -- show usage\n");
    return;
}

int Client_Run(Client* pstClient)
{
    if (pstClient == NULL) {
        return CLIENT_NULL;
    }
    if (strlen(pstClient->acName) < 1) {
        return CLIENT_NULL;
    }
    if (pstClient->state != CLIENT_RUNNING) {
        return CLIENT_ERR;
    }
    int ret;
    char acType[MESSAGE_TYPE_LEN];
    /* show usage */
    Client_ShowUsage();
    /* start recieve message */
    pstClient->state = CLIENT_RUNNING;
    /* create thread to receive message */
    ret = pthread_create(&pstClient->recvTid, NULL, Client_RecvMessage, (void*)pstClient);
    if (ret < 0) {
        CLIENT_LOG("fail to create thread");
        return CLIENT_THREAD_ERR;
    }
    /* show usage */
    Client_SendMessage(pstClient, "REGISTER");
    /* register and login */
    while (1) {
        memset(acType, 0, MESSAGE_TYPE_LEN);
        printf("%s", "send >");
        scanf("%s", acType);
        if (strcmp(acType, "exit") == 0) {
            /* logout */
            Client_SendMessage(pstClient, "LOGOUT");
            pthread_mutex_lock(&pstClient->stLock);
            pstClient->state = CLIENT_SHUTDOWN;
            pthread_mutex_unlock(&pstClient->stLock);
            break;
        } else if (strcmp(acType, "usage") == 0) {
            Client_ShowUsage();
        }
        Client_SendMessage(pstClient, acType);
        /* exit */
        pthread_mutex_lock(&pstClient->stLock);
        if (pstClient->state == CLIENT_SHUTDOWN) {
            pthread_mutex_unlock(&pstClient->stLock);
            break;
        }
        pthread_mutex_unlock(&pstClient->stLock);
    }
    return CLIENT_OK;
}

int Client_Close(Client* pstClient)
{
    if (pstClient == NULL) {
        return CLIENT_NULL;
    }
    /* stop reveiving message */
    pthread_mutex_lock(&pstClient->stLock);
    pstClient->state = CLIENT_SHUTDOWN;
    pthread_mutex_unlock(&pstClient->stLock);
    /* recycle thread */
    pthread_join(pstClient->recvTid, NULL);
    /* close socket */
    close(pstClient->fd);
    return CLIENT_OK;
}

/* message */
void Client_SendMessage(Client* pstClient, char* pcType)
{
    if (pcType == NULL) {
        return;
    }
    int type = 0;
    int port;
    socklen_t len;
    struct sockaddr_in stAddr;
    char acMessage[MESSAGE_MAX_LEN] = {0};
    char acDstName[MESSAGE_NAME_LEN] = {0};
    char acContent[MESSAGE_CONTENT_LEN] = {0};
    char acIpAddr[INET_ADDRSTRLEN] = {0};
    /* parse type */
    type = g_pstIfMessage->pfParseType(pcType);
    /* create packet */
    switch (type) {
        case MESSAGE_TRANSFER:
            printf("%s", "DstName >");
            scanf("%s", acDstName);
            printf("%s", "Content >");
            scanf("%s", acContent);
            g_pstIfMessage->pfCreateTransferMessage(type, pstClient->acName, acDstName, acContent, acMessage);
            break;
        case MESSAGE_BROADCAST:   
            printf("%s", "Content >");
            scanf("%s", acContent);
            g_pstIfMessage->pfCreateBroadcastMessage(type, pstClient->acName, acContent, acMessage);
            break;
        case MESSAGE_GETALLUSER:  
        case MESSAGE_LOGOUT:      
        case MESSAGE_APPADMIN:    
        case MESSAGE_REFLECT:     
        case MESSAGE_SHUTDOWN:     
            g_pstIfMessage->pfCreateMessageWithName(type, pstClient->acName, acMessage);
            break;
        case MESSAGE_KICKOUT:     
        case MESSAGE_BANTALK:     
            printf("%s", "DstName >");
            scanf("%s", acDstName);
            g_pstIfMessage->pfCreateMessageWithDirect(type, pstClient->acName, acDstName, acMessage);
            break;
        case MESSAGE_REGISTER:
            /* get info from socket */
            len = sizeof(stAddr);
            getsockname(pstClient->fd, (struct sockaddr*)&stAddr, &len);
            /* get ip */
            inet_ntop(AF_INET, &stAddr.sin_addr, acIpAddr, INET_ADDRSTRLEN);
            /* get port */
            port = ntohs(stAddr.sin_port);
            g_pstIfMessage->pfCreateRegisterMessage(type, pstClient->acName, acIpAddr, port, acMessage);
            break;
        default:
            g_pstIfMessage->pfCreateMessageWithName(type, pstClient->acName, acMessage);
            break;
    }
    /* send packet */
    printf("create packet: %s\n", acMessage);
    send(pstClient->fd, acMessage, strlen(acMessage), 0);
    return;
}

void* Client_RecvMessage(void* pvArg)
{
    Client* pstClient = (Client*)pvArg;
    char acMessage[MESSAGE_MAX_LEN];
    int type;
    ssize_t len;
    while (1) {
        memset(acMessage, 0, MESSAGE_MAX_LEN);
        len = recv(pstClient->fd, acMessage, MESSAGE_MAX_LEN, 0);
        if (len > 0) {
            pthread_mutex_lock(&pstClient->stLock);
            printf("\nrecv message: %s\n", acMessage);
            /* stop reveiving message */
            type = g_pstIfMessage->pfGetType(acMessage);
            if (type == MESSAGE_SHUTDOWN) {
                Client_SendMessage(pstClient, "LOGOUT");
                pstClient->state = CLIENT_SHUTDOWN;
            }
            if (type == MESSAGE_LOGOUT) {
                pstClient->state = CLIENT_LOGOUT;
            }
            if (pstClient->state == CLIENT_SHUTDOWN) {
                pthread_mutex_unlock(&pstClient->stLock);
                break;
            }
            pthread_mutex_unlock(&pstClient->stLock);
        }
    }
    return NULL;
}
