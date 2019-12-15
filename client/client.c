#include "client.h"
MessageInterface* g_pstIfMessage = &g_stIfMessage;
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
    pthread_mutex_init(&pstClient->stLock, NULL);
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
    /* connect */
    struct sockaddr_in stAddr;
    stAddr.sin_family = AF_INET;
    stAddr.sin_port = htons(port);
    stAddr.sin_addr.s_addr = inet_addr(pcIpAddr);
    ret = connect(pstClient->fd, (struct sockaddr*)&stAddr, sizeof(stAddr));
    if (ret < 0) {
        CLIENT_LOG("fail to connect server");
        close(pstClient->fd);
        return CLIENT_CONNECT_ERR;
    }
    /* create thread to receive message */
    pstClient->state = CLIENT_RUNNING;
    ret = pthread_create(&pstClient->recvTid, NULL,Client_RecvMessage, (void*)pstClient);
    if (ret < 0) {
        CLIENT_LOG("fail to create thread");
        close(pstClient->fd);
        return CLIENT_THREAD_ERR;
    }
    return CLIENT_OK;
}

int Client_Run(Client* pstClient)
{
    if (pstClient == NULL) {
        return CLIENT_NULL;
    }
    if (strlen(pstClient->acName) < 1) {
        return CLIENT_NULL;
    }
    char acType[MESSAGE_TYPE_LEN];
    while (1) {
        memset(acType, 0, MESSAGE_TYPE_LEN);
        printf("%s", "send >");
        scanf("%s", acType);
        if (strcmp(acType, "exit") == 0) {
            break;
        }
        Client_SendMessage(pstClient, acType);
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
    /* logout */
    Client_SendMessage(pstClient, "LOGOUT");
    /* close socket */
    close(pstClient->fd);
    return CLIENT_OK;
}

/* configure */
int Client_ReadConfig(Client* pstClient, char* pcFileName)
{

    if (pstClient == NULL || pcFileName == NULL) {
        return CLIENT_NULL;
    }
    char acBuffer[128] = {0};
    char* pcName = NULL;
    /* ClientName:Tom */
    FILE* pstFile = NULL;
    if (pstFile == NULL) {
        CLIENT_LOG("fail to read config file");
        return CLIENT_NO_FILE;
    }
    fgets(acBuffer, 128, pstFile);
    /* client name */
    pcName = strstr(acBuffer, "ClientName:");
    if (pcName == NULL) {
        fclose(pstFile);
        return CLIENT_NOT_FOUND;
    }
    strcpy(pstClient->acName, pcName);
    fclose(pstFile);
    return CLIENT_OK;
}

/* message */
void Client_SendMessage(Client* pstClient, char* pcType)
{
    if (pcType == NULL) {
        return;
    }
    int type = 0;
    char* pcIpAddr = NULL;
    int port;
    struct sockaddr_in stAddr;
    char acMessage[MESSAGE_MAX_LEN] = {0};
    char acDstName[MESSAGE_NAME_LEN] = {0};
    char acContent[MESSAGE_CONTENT_LEN] = {0};
    /* parse type */
    g_pstIfMessage->pfParseType(pcType);
    /* create packet */
    switch (type) {
        case MESSAGE_TRANSFER:
            printf("%s", "DstName >");
            scanf("%s", acDstName);
            printf("%s", "\n");
            printf("%s", "Content >");
            scanf("%s", acContent);
            printf("%s", "\n");
            g_pstIfMessage->pfCreateTransferMessage(type, pstClient->acName, acDstName, acContent, acMessage);
            break;
        case MESSAGE_BROADCAST:   
            printf("%s", "Content >");
            scanf("%s", acContent);
            printf("%s", "\n");
            g_pstIfMessage->pfCreateBroadcastMessage(type, pstClient->acName, acContent, acMessage);
            break;
        case MESSAGE_GETALLUSER:  
        case MESSAGE_LOGOUT:      
        case MESSAGE_APPADMIN:    
        case MESSAGE_REFLECT:     
            g_pstIfMessage->pfCreateMessageWithName(type, pstClient->acName, acMessage);
            break;
        case MESSAGE_KICKOUT:     
        case MESSAGE_BANTALK:     
            printf("%s", "DstName >");
            scanf("%s", acDstName);
            printf("%s", "\n");
            g_pstIfMessage->pfCreateMessageWithDirect(type, pstClient->acName, acDstName, acMessage);
            break;
        case MESSAGE_REGISTER:
            /* get info from socket */
            getsockname(pstClient->fd, (struct sockaddr*)&stAddr, NULL);
            /* get ip */
            pcIpAddr = inet_ntoa(stAddr.sin_addr);
            /* get port */
            port = ntohs(stAddr.sin_port);
            g_pstIfMessage->pfCreateRegisterMessage(type, pstClient->acName, pcIpAddr, port, acMessage);
            break;
        default:
            g_pstIfMessage->pfCreateMessageWithName(type, pstClient->acName, acMessage);
            break;
    }
    /* send packet */
    send(pstClient->fd, acMessage, strlen(acMessage), 0);
    return;
}

void* Client_RecvMessage(void* pvArg)
{
    Client* pstClient = (Client*)pvArg;
    char acMessage[MESSAGE_MAX_LEN];
    while (1) {
        /* stop reveiving message */
        pthread_mutex_lock(&pstClient->stLock);
        if (pstClient->state == CLIENT_SHUTDOWN) {
            pthread_mutex_unlock(&pstClient->stLock);
            break;
        }
        pthread_mutex_unlock(&pstClient->stLock);
        memset(acMessage, 0, MESSAGE_MAX_LEN);
        recv(pstClient->fd, acMessage, MESSAGE_MAX_LEN, 0);
        printf("recv message: %s\n", acMessage);
    }
    return NULL;
}
