#include "message.h"
const char* g_pcMessageType[MESSAGE_TYPE_NUM] = {
    "TRANSFER",
    "BROADCAST",
    "GETALLUSER",
    "LOGOUT",
    "KICKOUT",
    "BANTALK",
    "ADMIN",
    "REFLECT",
    "REGISTER",
    "SHUTDOWN"
};

MessageInterface g_stIfMessage = {
    Message_CreateTransferMessage,
    Message_CreateBroadcastMessage,
    Message_CreateRegisterMessage,
    Message_CreateMessageWithName,
    Message_CreateMessageWithDirect,
    Message_DeleteAdapter,
    Message_GetType,
    Message_GetContentLength,
    Message_GetSrcName,
    Message_GetDstName,
    Message_GetIpAddr,
    Message_GetPort,
    Message_GetString,
    Message_GetInt,
    Message_ParseType
};

char* Message_New(int type, const char* pcSrcName, const char* pcSrcIpAddr,
        const char* pcDstName, const char* pcContent)
{
    if (pcSrcName == NULL || pcSrcIpAddr == NULL ||
            pcDstName == NULL || pcContent == NULL) {
        return NULL;
    }
    char* pcMessage = NULL;
    pcMessage = (char*)malloc(MESSAGE_MAX_LEN);
    if (pcMessage == NULL) {
        return NULL;
    }
    sprintf(pcMessage,
            "type:%d,srcName:%s,srcIpAddr:%s,dstName:%s,content:%s",
            type, pcSrcName, pcSrcIpAddr, pcDstName, pcContent);
    return pcMessage;
}

int Message_CreateTransferMessage(int type, char* pcSrcName, char* pcDstName, char* pcContent, char* pcMessage)
{
    sprintf(pcMessage,
            "type:%d,srcName:%s,dstName:%s,content:%s",
            type, pcSrcName, pcDstName, pcContent);
    return MESSAGE_OK;
}

int Message_CreateBroadcastMessage(int type, char* pcName, char* pcContent, char* pcMessage)
{
    sprintf(pcMessage,
            "type:%d,srcName:%s,content:%s",
            type, pcName, pcContent);
    return MESSAGE_OK;
}

int Message_CreateRegisterMessage(int type, char* pcName, char* pcIpAddr, int port, char* pcMessage)
{
    sprintf(pcMessage,
            "type:%d,srcName:%s,ip:%s,port:%d",
            type, pcName, pcIpAddr, port);
    return MESSAGE_OK;
}

int Message_CreateMessageWithName(int type, char* pcName, char* pcMessage)
{
    sprintf(pcMessage,
            "type:%d,srcName:%s",
            type, pcName);
    return MESSAGE_OK;
}

int Message_CreateMessageWithDirect(int type, char* pcSrcName, char* pcDstName, char* pcMessage)
{
    sprintf(pcMessage,
            "type:%d,srcName:%s,dstName:%s",
            type, pcSrcName, pcDstName);
    return MESSAGE_OK;
}

int Message_GetString(char* pcMessage, char* pcField, char* pcString)
{
    if (pcMessage == NULL || pcField == NULL || pcString == NULL) {
        return MESSAGE_NULL;
    }
    char* pcBegin = NULL;
    char* pcStr = NULL;
    char acTmp[MESSAGE_MAX_LEN];
    strcpy(acTmp, pcMessage);
    pcBegin = strstr(acTmp, pcField);
    if (pcBegin == NULL) {
        return MESSAGE_NOT_FOUND;
    }
    pcStr = strtok(pcBegin + strlen(pcField), ",");
    if (pcStr == NULL) {
        return MESSAGE_NOT_FOUND;
    }
    strcpy(pcString, pcStr);
    return MESSAGE_OK;
}

int Message_GetInt(char* pcMessage, char* pcField)
{
    if (pcMessage == NULL || pcField == NULL) {
        return -1;
    }
    char* pcBegin = NULL;
    char* pcInt = NULL;
    int num = 0;
    char acTmp[MESSAGE_MAX_LEN];
    strcpy(acTmp, pcMessage);
    pcBegin = strstr(acTmp, pcField);
    if (pcBegin == NULL) {
        return -1;
    }
    pcInt = strtok(pcBegin + strlen(pcField), ",");
    if (pcInt == NULL) {
        return -1;
    }
    num = atoi(pcInt);
    return num;
}

int Message_GetType(char* pcMessage)
{
    return Message_GetInt(pcMessage, "type:");
}

int Message_GetContentLength(char* pcMessage)
{
    return Message_GetInt(pcMessage, "len:");
}

int Message_GetDstName(char* pcMessage, char* pcDstName)
{
    return Message_GetString(pcMessage, "dstName:", pcDstName);
}

int Message_GetSrcName(char* pcMessage, char* pcSrcName)
{
    return Message_GetString(pcMessage, "srcName:", pcSrcName);
}

int Message_GetIpAddr(char* pcMessage, char* pcIpAddr)
{
    return Message_GetString(pcMessage, "ip:", pcIpAddr);
}

int Message_GetPort(char* pcMessage)
{
    return Message_GetInt(pcMessage, "port:");
}

int Message_ParseType(char* pcType)
{
    if (pcType == NULL) {
        return -1;
    }
    int type = 0;
    int i;
    for (i = 0; i < MESSAGE_TYPE_NUM; i++) {
        if (strcmp(g_pcMessageType[i], pcType) == 0) {
            type = i;
            break;
        }
    }
    return type;
}
/* adapter */
int Message_DeleteAdapter(void* pvMessage)
{
    free(pvMessage);
    return MESSAGE_OK;
}
