#ifndef MESSAGE_H
#define MESSAGE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MESSAGE_MAX_LEN     1200
#define MESSAGE_NAME_LEN    64
#define MESSAGE_CONTENT_LEN 1024
#define MESSAGE_TRANSFER    0
#define MESSAGE_BROADCAST   1
#define MESSAGE_GETALLUSER  2
#define MESSAGE_LOGOUT      3
#define MESSAGE_KICKOUT     4
#define MESSAGE_BANTALK     5
#define MESSAGE_APPADMIN    6
#define MESSAGE_REFLECT     7
#define MESSAGE_REGISTER    8
#define MESSAGE_SHUTDOWN    9

#define MESSAGE_TYPE_NUM     10
#define MESSAGE_TYPE_LEN     32
const char* g_pcMessageType[MESSAGE_TYPE_NUM];

typedef enum MESSAGE_ENUM {
    MESSAGE_OK,					
    MESSAGE_ERR,				
    MESSAGE_NULL,		
    MESSAGE_MEM_ERR,		
    MESSAGE_SIZE_ERR,
    MESSAGE_NOT_FOUND,
    MESSAGE_NO_FILE
} MESSAGE_ENUM;
/* transfer : 
type:0,srcName:tom,dstName:jen,content:hello */
/* broadcast : 
type:1,name:tom,content:hello */
/* getalluser : 
type:2,name:tom, */
/* logout : 
type:3,name:tom, */
/* kickout : 
type:4,srcName:tom,dstName:jen, */
/* bantalk : 
type:5,srcName:tom,dstName:jen, */
/* applyforadministrator : 
type:6,name:tom, */
/* reflect : 
type:7,name:tom, */
/* register: 
type:8,name:tom,ip:192.168.1.3,port:8011, */
/* shutdown : 
type:9,name:tom, */
typedef struct MessageInterface {
    int (*pfCreateTransferMessage)(int type, char* pcSrcName, char* pcDstName, char* pcContent, char* pcMessage);
    int (*pfCreateBroadcastMessage)(int type, char* pcName, char* pcContent, char* pcMessage); 
    int (*pfCreateRegisterMessage)(int type, char* pcName, char* pcIpAddr, int port, char* pcMessage);
    int (*pfCreateMessageWithName)(int type, char* pcName, char* pcMessage);
    int (*pfCreateMessageWithDirect)(int type, char* pcSrcName, char* pcDstName, char* pcMessage);
    int (*pfDelete)(void* pvMessage);
    int (*pfGetType)(char* pcMessage);
    int (*pfGetContentLength)(char* pcMessage);
    int (*pfGetSrcName)(char* pcMessage, char* pcSrcName);
    int (*pfGetDstName)(char* pcMessage, char* pcDstName);
    int (*pfGetIpAddr)(char* pcMessage, char* pcSrcIpAddr);
    int (*pfGetPort)(char* pcMessage);
    int (*pfGetString)(char* pcMessage, char* pcField, char* pcString);
    int (*pfGetInt)(char* pcMessage, char* pcField);
    int (*pfParseType)(char* pcType);
} MessageInterface;

MessageInterface g_stIfMessage;

int Message_CreateTransferMessage(int type, char* pcSrcName, char* pcDstName, char* pcContent, char* pcMessage);
int Message_CreateBroadcastMessage(int type, char* pcName, char* pcContent, char* pcMessage); 
int Message_CreateRegisterMessage(int type, char* pcName, char* pcIpAddr, int port, char* pcMessage);
int Message_CreateMessageWithName(int type, char* pcName, char* pcMessage);
int Message_CreateMessageWithDirect(int type, char* pcSrcName, char* pcDstName, char* pcMessage);
int Message_Delete(char* pcMessage);
int Message_GetType(char* pcMessage);
int Message_GetContentLength(char* pcMessage);
int Message_GetSrcName(char* pcMessage, char* pcSrcName);
int Message_GetDstName(char* pcMessage, char* pcDstName);
int Message_GetIpAddr(char* pcMessage, char* pcSrcIpAddr);
int Message_GetPort(char* pcMessage);
int Message_GetString(char* pcMessage, char* pcField, char* pcString);
int Message_GetInt(char* pcMessage, char* pcField);
int Message_ParseType(char* pcType);
/* adapter */
int Message_DeleteAdapter(void* pvMessage);

#endif // MESSAGE_H
