#ifndef HASHMAP_H
#define HASHMAP_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef enum HASHMAP_ENUM {
    HASHMAP_OK,
    HASHMAP_ERR,
    HASHMAP_NULL,
    HASHMAP_MEM_ERR,
    HASHMAP_SIZE_ERR,
    HASHMAP_NOT_FOUND,
    HASHMAP_NO_FILE
} HASHMAP_ENUM;
#define HASHMAP_BUFFER_LEN 128
#define HASHMAP_MESSAGE(message) do { \
    printf("file : %s    line : %d    message : %s", __FILE__, __LINE__, (message)); \
} while (0)
/* hash node */
typedef struct HashNode {
    char* pcKey;
    void* pvInstance;
    struct HashNode *pstNext;
} HashNode;
/* string hash map */
typedef struct HashMap {
    HashNode* *ppstHashTable;
    int nodeNum;
    int maxListNum;
    void* (*pfObject_New)();
    int (*pfObject_Delete)(void* pvInstance);
    int (*pfObject_Compare)(void* pvInstance1, void* pvInstance2);
    int (*pfObject_Write)(FILE* pstFile, void* pvInstance);
    int (*pfObject_Read)(FILE* pstFile, void* pvInstance);
} HashMap;
/* interface */
typedef struct HashMapInterface {
    HashMap* (*pfCreate)(int maxListNum);
    int (*pfDestroy)(HashMap* pstHashMap);
    int (*pfRegisterObject)(
            HashMap* pstHashMap,
            void* (*pfObject_New)(),
            int (*pfObject_Delete)(void* pvInstance),
            int (*pfObject_Compare)(void* pvInstance1, void* pvInstance2),
            int (*pfObject_Write)(FILE* pstFile, void* pvInstance),
            int (*pfObject_Read)(FILE* pstFile, void* pvInstance));
    int (*pfInsert)(HashMap* pstHashMap, const char* pcKey, void* pvInstance);
    int (*pfDelete)(HashMap* pstHashMap, const char* pcKey);
    int (*pfSearch)(HashMap* pstHashMap, const char* pcKey, void** ppvInstance);
    int (*pfTraverse)(HashMap* pstHashMap, int (*pfCallBack)(HashNode* pstNode, void* pArg), void* pArg);
    int (*pfResize)(HashMap* pstHashMap, int iNewSize);
    int (*pfLoad)(HashMap* pstHashMap, const char* pcFileName);
    int (*pfSave)(HashMap* pstHashMap, const char* pcFileName);
} HashMapInterface;

HashMapInterface g_stHashMapInterface;

HashNode* HashMap_CreateNode(const char* pcKey, void* pvInstance);
void HashMap_DestroyNode(HashNode* pstHashNode, int (*pfObject_Delete)(void* pvInstance));
unsigned int HashMap_Func(const char* pcKey);
HashMap* HashMap_Create(int maxListNum);
int HashMap_Destroy(HashMap* pstHashMap);
int HashMap_RegisterObject(
        HashMap* pstHashMap,
        void* (*pfObject_New)(),
        int (*pfObject_Delete)(void* pvInstance),
        int (*pfObject_Compare)(void* pvInstance1, void* pvInstance2),
        int (*pfObject_Write)(FILE* pstFile, void* pvInstance),
        int (*pfObject_Read)(FILE* pstFile, void* pvInstance));
int HashMap_Insert(HashMap* pstHashMap, const char* pcKey, void* pvInstance);
int HashMap_Delete(HashMap* pstHashMap, const char* pcKey);
int HashMap_Search(HashMap* pstHashMap, const char* pcKey, void** ppvInstance);
int HashMap_Traverse(HashMap* pstHashMap, int (*pfCallBack)(HashNode* pstNode, void* pArg), void* pArg);
int HashMap_Resize(HashMap* pstHashMap, int iNewSize);
int HashMap_Load(HashMap* pstHashMap, const char* pcFileName);
int HashMap_Save(HashMap* pstHashMap, const char* pcFileName);
#endif // HASHMAP
