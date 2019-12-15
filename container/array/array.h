#ifndef ARRAY_H
#define ARRAY_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __cplusplus
extern "C" {
#endif
/* debug message */
#define ARRAY_MESSAGE(message) do { \
    printf("file: %s  function: %s  line: %d  message: %s\n", \
            __FILE__, __FUNCTION__, __LINE__, (message)); \
} while (0)
#define ARRAY_ASSERT(condition, errcode) do { \
    if (!(condition)) { \
        printf("file: %s  function: %s  line: %d  errcode: %d\n", \
                __FILE__, __FUNCTION__, __LINE__, (errcode)); \
    } \
} while (0)
typedef enum ARRAY_ENUM{
    ARRAY_OK,				
    ARRAY_ERR,				
    ARRAY_NULL,				
    ARRAY_SIZE_ERR,			
    ARRAY_MEM_ERR,			
    ARRAY_NOT_FOUND,
    ARRAY_NO_FILE			
} ARRAY_ENUM;
/* array */
typedef struct Array {
    void** ppvInstanceTable;
    int iCurrentNum;
    int iMaxNum;
    void* (*pfObject_New)();
    int (*pfObject_Delete)(void* pvInstance);
    int (*pfObject_Compare)(void* pvInstance1, void* pvInstance2);
    int (*pfObject_Write)(FILE* pstFile, void* pvInstance);
    int (*pfObject_Read)(FILE* pstFile, void* pvInstance);
} Array;
/* interface */
typedef struct ArrayInterface {
    Array* (*pfNew)(int iMaxNum);
    int (*pfDelete)(Array* pstArray);
    int (*pfRegisterObject)(Array* pstArray,
            void* (*pfObject_New)(),
            int (*pfObject_Delete)(void* pvInstance),
            int (*pfObject_Compare)(void* pvInstance1, void* pvInstance2),
            int (*pfObject_Write)(FILE* pstFile, void* pvInstance),
            int (*pfObject_Read)(FILE* pstFile, void* pvInstance));
    int (*pfResize)(Array* pstArray, int newSize);
    int (*pfPushBack)(Array* pstArray, void* pvInstance);
    int (*pfPushFront)(Array* pstArray, void* pvInstance);
    int (*pfPopBack)(Array* pstArray);
    int (*pfPopFront)(Array* pstArray);
    int (*pfSearch)(Array* pstArray, void* pvInstance);
    int (*pfInsertByIndex)(Array* pstArray, int index, void* pvInstance);
    int (*pfInsertByValue)(Array* pstArray, void* pvPreInstance, void* pvInstance);
    int (*pfDeleteByIndex)(Array* pstArray, int index);
    int (*pfDeleteByValue)(Array* pstArray, void* pvInstance);
    int (*pfTraverse)(Array* pstArray, int (*pfCallBack)(void* pvInstance, void* pvArg), void* pvArg);
    int (*pfSort)(Array* pstArray);
    int (*pfSave)(Array* pstArray, const char* fileName);
    int (*pfLoad)(Array* pstArray, const char* fileName);
} ArrayInterface;

ArrayInterface g_stArrayInterface;

/* new array */
Array* Array_New(int iMaxNum);
/* delete */
int Array_Delete(Array* pstArray);
/* register object */
int Array_RegisterObject(Array* pstArray,
        void* (*pfObject_New)(),
        int (*pfObject_Delete)(void* pvInstance),
        int (*pfObject_Compare)(void* pvInstance1, void* pvInstance2),
        int (*pfObject_Write)(FILE* pstFile, void* pvInstance),
        int (*pfObject_Read)(FILE* pstFile, void* pvInstance));
/* resize */
int Array_Resize(Array* pstArray, int newSize);
/* push back */
int Array_PushBack(Array* pstArray, void* pvInstance);
/* push front */
int Array_PushFront(Array* pstArray, void* pvInstance);
/* pop back */
int Array_PopBack(Array* pstArray);
/* pop front */
int Array_PopFront(Array* pstArray);
/* search return index */
int Array_Search(Array* pstArray, void* pvInstance);
/* insert by index */
int Array_InsertByIndex(Array* pstArray, int index, void* pvInstance);
/* insert by pvInstance */
int Array_InsertByValue(Array* pstArray, void* pvPreInstance, void* pvInstance);
/* delete by index */
int Array_DeleteByIndex(Array* pstArray, int index);
/* delete by pvInstance */
int Array_DeleteByValue(Array* pstArray, void* pvInstance);
/* traverse */
int Array_Traverse(Array* pstArray, int (*pfCallBack)(void* pvInstance, void* pvArg), void* pvArg);
/* sort */
int Array_Sort(Array* pstArray);
/* save */
int Array_Save(Array* pstArray, const char* fileName);
/* load */
int Array_Load(Array* pstArray, const char* fileName);
#ifdef __cplusplus
}
#endif // __cplusplus
#endif // ARRAY_H
