#ifndef USER_H
#define USER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define USER_MAXBUFFERLEN  2048
#define USER_ADDRSTRLEN    16
#define USER_NAMELEN       64
#define USER_ONLINE        0
#define USER_LEAVE         1
#define USER_LOGOUT        2
#define USER_KICKOUT       3
#define USER_BANTALK       4
#define USER_NONADMIN      0
#define USER_ADMIN         1
typedef enum USER_ENUM {
	USER_OK,					
	USER_ERR,				
	USER_NULL,		
	USER_MEM_ERR,		
	USER_SIZE_ERR,
	USER_NOT_FOUND,
	USER_NO_FILE
} USER_ENUM;

typedef struct User {
    int fd;
    int port;
    int state;
    int stat;
    int isRegistered;
    char acIpAddr[USER_ADDRSTRLEN];
    char acName[USER_NAMELEN];
} User;

User* User_New(int fd, int port, int state, int stat, char* pcIpAddr, char* pcName);
int User_Delete(User* pstUser);
int User_ToString(User* pstUser, char* pcString, int len);
int User_Parse(User* pstUser, char* pcString);
int User_Compare(User* pstUser1, User* pstUser2);
int User_Write(FILE* pstFile, User* pstUser);
int User_Read(FILE* pstFile, User* pstUser);
/* adapter */
void* User_NewAdapter();
int User_DeleteAdapter(void* pvInstance);
int User_CompareAdapter(void* pvInstance1, void* pvInstance2);
int User_WriteAdapter(FILE* pstFile, void* pvInstance);
int User_ReadAdapter(FILE* pstFile, void* pvInstance);
#endif // USER_H
