#ifndef List_H
#define List_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum LIST_ENUM {
     LIST_OK,					
     LIST_ERR,				
     LIST_NULL,		
     LIST_MEM_ERR,		
     LIST_SIZE_ERR,
     LIST_NOT_FOUND,
     LIST_NO_FILE
} LIST_ENUM;

#define LIST_MESSAGE(message) do { \
    printf("file: %s  function: %s  line: %d  message: %s\n", \
            __FILE__, __FUNCTION__, __LINE__, (message)); \
} while (0)
/* node */
typedef struct Node {
    void* pvInstance;
    struct Node *pstPre;
    struct Node *pstNext;
} Node;
/* double list */
typedef struct List {
    struct Node *pstHead;
    struct Node *pstTail;
    int iNodeNum;
    void* (*pfObject_New)();
    int (*pfObject_Delete)(void* pvInstance);
    int (*pfObject_Compare)(void* pvInstance1, void* pvInstance2);
    int (*pfObject_Write)(FILE* pstFile, void* pvInstance);
    int (*pfObject_Read)(FILE* pstFile, void* pvInstance);
} List;
/* interface */
typedef struct ListInterface {
    List* (*pfNew)();
    int (*pfRegisterObject)(List* pstList,
            void* (*pfObject_New)(),
            int (*pfObject_Delete)(void* pvInstance),
            int (*pfObject_Compare)(void* pvInstance1, void* pvInstance2),
            int (*pfObject_Write)(FILE* pstFile, void* pvInstance),
            int (*pfObject_Read)(FILE* pstFile, void* pvInstance));
    int (*pfDelete)(List *pstList);
    Node* (*pfSearch)(const List *pstList, void *pvInstance);
    Node* (*pfGetFront)(List* pstList);
    int (*pfPushBack)(List *pstList, void* pvInstance);
    int (*pfPopBack)(List *pstList);
    int (*pfPushFront)(List *pstList, void* pvInstance);
    int (*pfPopFront)(List *pstList);
    int (*pfInsertByIndex)(List *pstList, void* pvInstance, int index);
    int (*pfDeleteByIndex)(List *pstList, int index);
    int (*pfInsertByValue)(List *pstList, void* pvPreInstance, void* pvInstance);
    int (*pfDeleteByValue)(List *pstList, void* pvInstance);
    int (*pfSort)(List *pstList);
    int (*pfReverse)(List *pstList);
    int (*pfTraverse)(const List *pstList, int (*pfCallBack)(void* pvInstance, void* pvArg), void* pvArg);
    int (*pfSave)(List *pstList, const char* pcFileName);
    int (*pfLoad)(List *pstList, const char* pcFileName);
} ListInterface;

ListInterface g_stIfList;

/* new List */
List* List_New();
/* register object */
int List_RegisterObject(List* pstList,
        void* (*pfObject_New)(),
        int (*pfObject_Delete)(void* pvInstance),
        int (*pfObject_Compare)(void* pvInstance1, void* pvInstance2),
        int (*pfObject_Write)(FILE* pstFile, void* pvInstance),
        int (*pfObject_Read)(FILE* pstFile, void* pvInstance));
/* delete List */
int List_Delete(List *pstList);
/* search */
Node* List_Search(const List *pstList, void *pvInstance);
/* get */
Node* List_GetFront(List* pstList);
/* push back */
int List_PushBack(List *pstList, void* pvInstance);
/* pop back */
int List_PopBack(List *pstList);
/* push front */
int List_PushFront(List *pstList, void* pvInstance);
/* pop front */
int List_PopFront(List *pstList);
/* insert by index */
int List_InsertByIndex(List *pstList, void* pvInstance, int index);
/* delete by index */
int List_DeleteByIndex(List *pstList, int index);
/* insert by Value */
int List_InsertByValue(List *pstList, void* pvPreInstance, void* pvInstance);
/* delete by Value */
int List_DeleteByValue(List *pstList, void* pvInstance);
/* sort */
int List_Sort(List *pstList);
/* reverse */
int List_Reverse(List *pstList);
/* traverse */
int List_Traverse(const List *pstList, int (*pfCallBack)(void* pvInstance, void* pvArg), void* pvArg);
/* write to file */
int List_Save(List *pstList, const char* pcFileName);
/* read from file */
int List_Load(List *pstList, const char* pcFileName);
#ifdef __cplusplus
}
#endif// __cplusplus
#endif// List_H
