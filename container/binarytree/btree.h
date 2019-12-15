#ifndef BTREE_H
#define BTREE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __cplusplus
extern "C" {
#endif
#define BTREE_MESSAGE(message) do { \
    printf("file: %s  function: %s  line: %d  message: %s\n", \
            __FILE__, __FUNCTION__, __LINE__, (message)); \
} while (0)
typedef enum BTREE_ENUM{
    BTREE_OK,					
    BTREE_ERR,				
    BTREE_NULL,		
    BTREE_MEM_ERR,		
    BTREE_SIZE_ERR,
    BTREE_NOT_FOUND,
    BTREE_NO_FILE
} BTREE_ENUM;
/* node */
typedef struct TriNode {
    void* pvInstance;
    struct TriNode* pstUp;
    struct TriNode* pstLeft;
    struct TriNode* pstRight;
} TriNode;
/* binary tree */
typedef struct BTree {
    TriNode* pstRoot;
    int iNodeNum;
    void* (*pfObject_New)();
    int (*pfObject_Delete)(void* pvInstance);
    int (*pfObject_Compare)(void* pvInstance1, void* pvInstance2);
    int (*pfObject_Write)(FILE* pstFile, void* pvInstance);
    int (*pfObject_Read)(FILE* pstFile, void* pvInstance);
} BTree;
/* interface */
typedef struct BTreeInterface {
    BTree* (*pfNew)();
    int (*pfRegisterObject)(BTree* pstBTree,
            void* (*pfObject_New)(),
            int (*pfObject_Delete)(void* pvInstance),
            int (*pfObject_Compare)(void* pvInstance1, void* pvInstance2),
            int (*pfObject_Write)(FILE* pstFile, void* pvInstance),
            int (*pfObject_Read)(FILE* pstFile, void* pvInstance));
    int (*pfDelete)(BTree* pstBTree);
    int (*pfInsert)(BTree* pstBTree, void* pvInstance);
    int (*pfRemove)(BTree* pstBTree, void* pvInstance);
    int (*pfTraverse)(BTree* pstBTree, int (*pfCallBack)(void* pvInstance1, void* pvInstance2), void* pvArg);
    int (*pfSearch)(BTree* pstBTree, void* pvInstance);
    int (*pfSave)(BTree* pstBTree, const char* pcFileName);
    int (*pfLoad)(BTree* pstBTree, const char* pcFileName);
} BTreeInterface;

BTreeInterface g_stBTreeInterface;

/* new */
BTree* BTree_New();
/* register object */
int BTree_RegisterObject(BTree* pstBTree,
        void* (*pfObject_New)(),
        int (*pfObject_Delete)(void* pvInstance),
        int (*pfObject_Compare)(void* pvInstance1, void* pvInstance2),
        int (*pfObject_Write)(FILE* pstFile, void* pvInstance),
        int (*pfObject_Read)(FILE* pstFile, void* pvInstance));
/* delete */
int BTree_Delete(BTree* pstBTree);
/* insert */
int BTree_Insert(BTree* pstBTree, void* pvInstance);
/* remove */
int BTree_Remove(BTree* pstBTree, void* pvInstance);
/* traverse */
int BTree_Traverse(BTree* pstBTree, int (*pfCallBack)(void* pvInstance1, void* pvInstance2), void* pvArg);
/* search */
int BTree_Search(BTree* pstBTree, void* pvInstance);
/* write to file */
int BTree_Save(BTree* pstBTree, const char* pcFileName);
/* read from file */
int BTree_Load(BTree* pstBTree, const char* pcFileName);
#ifdef __cplusplus
}
#endif
#endif// BTREE_H
