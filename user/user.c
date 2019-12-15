#include "user.h"
User* User_New(int fd, int port, int state, int stat, char* pcIpAddr, char* pcName)
{
    if (fd < 0 || state < 0 || port < 0 || stat < 0 || pcIpAddr == NULL || pcName == NULL) {
        return NULL;
    }
    User* pstUser = NULL;
    pstUser = (User*)malloc(sizeof(User));
    if (pstUser == NULL) {
        return NULL;
    }
    pstUser->fd = fd;
    pstUser->port = port;
    pstUser->state = state;
    pstUser->stat = stat;
    pstUser->isRegistered = 0;
    strcpy(pstUser->acIpAddr, pcIpAddr);
    strcpy(pstUser->acName, pcName);
    return pstUser;
}

int User_Delete(User* pstUser)
{
    free(pstUser);
    return USER_OK;
}

int User_ToString(User* pstUser, char* pcString, int len)
{
    if (len > USER_MAXBUFFERLEN) {
        return USER_SIZE_ERR;
    }
    if (pcString == NULL) {
        return USER_NULL;
    }
    if (pstUser == NULL) {
        return USER_NULL;
    }
    sprintf(pcString,
            "name:%s,ip:%s,port:%d,status:%d",
            pstUser->acName, pstUser->acIpAddr, pstUser->port, pstUser->stat);
    return USER_OK;
}

int User_Parse(User* pstUser, char* pcString)
{
    if (pcString == NULL) {
        return USER_NULL;
    }
    if (pstUser == NULL) {
        return USER_NULL;
    }
    char* p1 = NULL;
    char* p2 = NULL;
    /* name */
    p1 = strstr(pcString, "name:");
    if (p1 == NULL) {
        return USER_NULL;
    }
    p2 = strtok(p1, ",");
    if (p2 == NULL) {
        return USER_NULL;
    }
    strcpy(pstUser->acName, p2);
    /* ip */
    p1 = strstr(pcString, "ip:");
    if (p1 == NULL) {
        return USER_NULL;
    }
    p2 = strtok(p1, ",");
    if (p2 == NULL) {
        return USER_NULL;
    }
    strcpy(pstUser->acIpAddr, p2);
    /* port */
    p1 = strstr(pcString, "port:");
    if (p1 == NULL) {
        return USER_NULL;
    }
    p2 = strtok(p1, ",");
    if (p2 == NULL) {
        return USER_NULL;
    }
    pstUser->port = atoi(p2);
    /* status */
    p1 = strstr(pcString, "status:");
    if (p1 == NULL) {
        return USER_NULL;
    }
    pstUser->stat = atoi(p1);
    return USER_OK;
}

int User_Compare(User* pstUser1, User* pstUser2)
{
    return strcmp(pstUser1->acIpAddr, pstUser2->acIpAddr);
}

int User_Write(FILE* pstFile, User* pstUser)
{
    if (pstFile == NULL) {
        return USER_NO_FILE;
    }
    char acBuffer[USER_MAXBUFFERLEN] = {0};
    int ret;
    ret = User_ToString(pstUser, acBuffer, strlen(acBuffer));
    if (ret != USER_OK) {
        return ret;
    }
    fputs(acBuffer, pstFile);
    return USER_OK;
}

int User_Read(FILE* pstFile, User* pstUser)
{
    if (pstFile == NULL) {
        return USER_NO_FILE;
    }
    char acBuffer[USER_MAXBUFFERLEN] = {0};
    int ret;
    fgets(acBuffer, USER_MAXBUFFERLEN, pstFile);
    if (strlen(acBuffer) < 0) {
        return USER_NOT_FOUND;
    }
    ret = User_Parse(pstUser, acBuffer);
    if (ret != USER_OK) {
        return ret;
    }
    return USER_OK;
}

/* adapter */
void* User_NewAdapter()
{
    User* pstUser = NULL;
    pstUser = User_New(0, 0, 0, 0, "0.0.0.0", "unknow");
    return (void*)pstUser;
}

int User_DeleteAdapter(void* pvInstance)
{
    User* pstUser = (User*)pvInstance;
    return User_Delete(pstUser);
}

int User_CompareAdapter(void* pvInstance1, void* pvInstance2)
{
    if (pvInstance1 == NULL || pvInstance2 == NULL) {
        return USER_IP_LEN;
    }
    User* pstUser1 = (User*)pvInstance1;
    User* pstUser2 = (User*)pvInstance2;
    if (strcmp(pstUser1->acIpAddr, pstUser2->acIpAddr) == 0 &&
            pstUser1->port == pstUser2->port) {
        return 0;
    }
    return 1;
}

int User_WriteAdapter(FILE* pstFile, void* pvInstance)
{
    User* pstUser = (User*)pvInstance;
    return User_Write(pstFile, pstUser);
}

int User_ReadAdapter(FILE* pstFile, void* pvInstance)
{
    User* pstUser = (User*)pvInstance;
    return User_Read(pstFile, pstUser);
}
