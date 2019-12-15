#include "array.h"
/* init interface */
ArrayInterface g_stArrayInterface = {
    Array_New,
    Array_Delete,
    Array_RegisterObject,
    Array_Resize,
    Array_PushBack,
    Array_PushFront,
    Array_PopBack,
    Array_PopFront,
    Array_Search,
    Array_InsertByIndex,
    Array_InsertByValue,
    Array_DeleteByIndex,
    Array_DeleteByValue,
    Array_Traverse,
    Array_Sort,
    Array_Save,
    Array_Load
};
/* new array */
Array* Array_New(int iMaxNum)
{
	struct Array *pstArray = NULL;
	do {
		pstArray = (Array*)malloc(sizeof(Array));
		if (pstArray == NULL)	{
			ARRAY_MESSAGE("pstArray malloc failed");
			break;
		}
		void** ppvInstanceTable = (void**)malloc(sizeof(void*) * iMaxNum);
		if (ppvInstanceTable == NULL) {
			ARRAY_MESSAGE("ppvInstanceTable malloc failed");
			break;
		}
		int i;
		for (i = 0; i < iMaxNum; i++) {
			ppvInstanceTable[i] = NULL;
		}
		pstArray->ppvInstanceTable = ppvInstanceTable;
		pstArray->iCurrentNum = 0;
		pstArray->iMaxNum = iMaxNum;
		return pstArray;
	} while (0);
	if (pstArray != NULL) {
		free(pstArray);
	}
	return NULL;
}

/* delete */
int Array_Delete(Array* pstArray)
{
	if (pstArray== NULL) {
		ARRAY_MESSAGE("pstArray is null");
		return ARRAY_NULL;
	}
	if (pstArray->ppvInstanceTable == NULL) {
		ARRAY_MESSAGE("ppvInstanceTable is null");
		return ARRAY_NULL;
	}
	if (pstArray->pfObject_Delete == NULL) {
		ARRAY_MESSAGE("pfObject_Delete is null");
		return ARRAY_NULL;
	}
	int i;
	for(i = 0; i < pstArray->iMaxNum; i++) {
		if (pstArray->ppvInstanceTable[i] != NULL) {
			pstArray->pfObject_Delete(pstArray->ppvInstanceTable[i]);
		}
	}
	free(pstArray->ppvInstanceTable);
	free(pstArray);
	pstArray = NULL;
	return ARRAY_OK;
}

/* register object */
int Array_RegisterObject(Array* pstArray,
		void* (*pfObject_New)(),
	    int (*pfObject_Delete)(void* pvInstance),
	    int (*pfObject_Compare)(void* pvInstance1, void* pvInstance2),
	    int (*pfObject_Write)(FILE* pstFile, void* pvInstance),
	    int (*pfObject_Read)(FILE* pstFile, void* pvInstance))
{
	if (pstArray == NULL) {
		ARRAY_MESSAGE("empty pointer");
		return ARRAY_NULL;
	}
	pstArray->pfObject_New = pfObject_New;
	pstArray->pfObject_Delete = pfObject_Delete;
	pstArray->pfObject_Compare = pfObject_Compare;
	pstArray->pfObject_Write = pfObject_Write;
	pstArray->pfObject_Read = pfObject_Read;
	return ARRAY_OK;
}

/* resize */
int Array_Resize(Array* pstArray, int iMaxNum)
{
	if (pstArray == NULL || pstArray->ppvInstanceTable == NULL) {
		ARRAY_MESSAGE("pstArray or ppvInstanceTable is null");
		return ARRAY_NULL;
	}
	if (pstArray->iMaxNum >= iMaxNum) {
		ARRAY_MESSAGE("iMaxNum >= iMaxNum");
		return ARRAY_SIZE_ERR;
	}
	void** pvInstanceTable = NULL;
	pvInstanceTable = (void**)malloc(sizeof(void*) * iMaxNum);
	if (pvInstanceTable == NULL) {
		ARRAY_MESSAGE("pvInstanceTable malloc failed");
		return ARRAY_MEM_ERR;
	}
	int i;
	for (i = 0; i < iMaxNum; i++) {
		if (i < pstArray->iCurrentNum) {
			pvInstanceTable[i] = pstArray->ppvInstanceTable[i]; 
		} else {
			pvInstanceTable[i] = NULL;
		}
	}
	free(pstArray->ppvInstanceTable);
	pstArray->ppvInstanceTable = pvInstanceTable;
	pstArray->iMaxNum = iMaxNum;
	return ARRAY_OK;
}

/* push back */
int Array_PushBack(Array* pstArray, void* pvInstance)
{
	/* insert by index */
	return Array_InsertByIndex(pstArray, pstArray->iCurrentNum, pvInstance);
}

/* push front */
int Array_PushFront(Array* pstArray, void* pvInstance)
{
	/* insert by index */
	return Array_InsertByIndex(pstArray, 0, pvInstance);
}

/* pop back */
int Array_PopBack(Array* pstArray)
{
	/* delete by index */
	return Array_DeleteByIndex(pstArray, pstArray->iCurrentNum - 1);
}

/* pop front */
int Array_PopFront(Array* pstArray)
{
	/* delete by index */
	return Array_DeleteByIndex(pstArray, 0);
}

/* search return index */
int Array_Search(Array* pstArray, void* pvInstance)
{
	if (pstArray == NULL || pstArray->ppvInstanceTable == NULL || pvInstance == NULL) {
		ARRAY_MESSAGE("pstArray is null");
		return ARRAY_NULL;
	}
	if (pstArray->pfObject_Compare == NULL) {
		ARRAY_MESSAGE("pfObject_Compare is null");
		return ARRAY_NULL;
	}
	int i;
	for (i = 0; i < pstArray->iCurrentNum; i++) {
		if (pstArray->pfObject_Compare(pstArray->ppvInstanceTable[i], pvInstance) == 0) {
			return i;
		}
	}
	return -1;
}

/* insert by index */
int Array_InsertByIndex(Array* pstArray, int index, void* pvInstance)
{  
	if (pstArray == NULL || pstArray->ppvInstanceTable == NULL || pvInstance == NULL) {
		ARRAY_MESSAGE("pstArray is null");
		return ARRAY_NULL;
	}
	if (index < 0 || index > pstArray->iCurrentNum) {
		ARRAY_MESSAGE("index is out of border");
		return ARRAY_SIZE_ERR;
	}
	/* extend capacity */
	if (pstArray->iCurrentNum + 1 > pstArray->iMaxNum) {
		int ret = Array_Resize(pstArray, pstArray->iMaxNum * 2);
		if (ret != ARRAY_OK) {
			ARRAY_MESSAGE("resize fail");
			return ARRAY_MEM_ERR;
		}
	}
	/* shift */
	int i;
	for (i = pstArray->iCurrentNum - 1; i >= index; i--) {
		pstArray->ppvInstanceTable[i + 1] = pstArray->ppvInstanceTable[i];
	}
	/* insert */
	pstArray->ppvInstanceTable[index] = pvInstance;
	pstArray->iCurrentNum++;
	return ARRAY_OK;
}

/* insert by pvInstance */
int Array_InsertByValue(Array* pstArray, void* pvPreInstance, void* pvInstance)
{
	/* locate inserting point */
	int index = ARRAY_OK;
	index = Array_Search(pstArray, pvPreInstance);
	if (index < 0) {
		ARRAY_MESSAGE("pvPreInstance not exist");
		return ARRAY_NOT_FOUND;
	}
	/* insert by index */
	return Array_InsertByIndex(pstArray, index, pvInstance);

}

/* delete by index */
int Array_DeleteByIndex(Array* pstArray, int index)
{
	if (pstArray == NULL || pstArray->ppvInstanceTable == NULL) {
		ARRAY_MESSAGE("empty pointer ");
		return ARRAY_NULL;
	}
	if (index < 0 || index >= pstArray->iCurrentNum) {
		ARRAY_MESSAGE("index is out of border");
		return ARRAY_ERR;
	}
	if (pstArray->pfObject_Delete != NULL) {
		pstArray->pfObject_Delete(pstArray->ppvInstanceTable[index]);
	}
	int i;
	for (i = index; i < pstArray->iCurrentNum - 1; i++) {
		pstArray->ppvInstanceTable[i] = pstArray->ppvInstanceTable[i + 1];
	}
	pstArray->ppvInstanceTable[i] = NULL;
	pstArray->iCurrentNum--;
	return ARRAY_OK;
}

/* delete by pvInstance */
int Array_DeleteByValue(Array* pstArray, void* pvInstance)
{
	/* locate deleting point */
	int index = ARRAY_OK;
	index = Array_Search(pstArray, pvInstance);
	if (index < 0) {
		ARRAY_MESSAGE("pvInstance not exist");
		return ARRAY_NOT_FOUND;
	}
	/* delete by index */
	return Array_DeleteByIndex(pstArray, index);
}

int Array_Traverse(Array* pstArray, int (*pfCallBack)(void* pvInstance, void* pvArg), void* pvArg)
{
	if (pstArray == NULL || pstArray->ppvInstanceTable == NULL ) {
		ARRAY_MESSAGE("pstArray or ppvInstance is null");
		return ARRAY_NULL;
	}
	if (pfCallBack == NULL) {
		ARRAY_MESSAGE("call back fuction is not registered");
		return ARRAY_NULL;
	}
	int i;
	for (i = 0; i < pstArray->iCurrentNum; i++) {
		pfCallBack(pstArray->ppvInstanceTable[i], pvArg);
	}
	return ARRAY_OK;
}

/* quickSort */
static void Array_QuickSort(int first, int last, void** ppvInstanceTable, int (*pfObject_Compare)(void* pvInstance1, void* pvInstance2)) 
{
	if (first > last) {
		return;
	}
	int i = first;
	int j = last;
	void* pRef = ppvInstanceTable[first];
	while (i < j) {
		while (i < j && (pfObject_Compare(ppvInstanceTable[j], pRef) > 0)) {
			j--;
		}
		ppvInstanceTable[i] = ppvInstanceTable[j];
		while (i < j && (pfObject_Compare(ppvInstanceTable[i], pRef) < 0)) {
			i++;
		}
		ppvInstanceTable[j] = ppvInstanceTable[i];
	}
	ppvInstanceTable[i] = pRef;
	Array_QuickSort(first, i -1 , ppvInstanceTable, pfObject_Compare);
	Array_QuickSort(i + 1, last, ppvInstanceTable, pfObject_Compare);
	return;
}
/* sort */
int Array_Sort(Array* pstArray)
{
	if (pstArray == NULL || pstArray->ppvInstanceTable == NULL) {
		ARRAY_MESSAGE("pstArray or ppvInstanceTable is NULL");
		return ARRAY_NULL;
	}
	if (pstArray->pfObject_Compare == NULL) {
		ARRAY_MESSAGE("pfObject_Compare fuction is not registered");
		return ARRAY_NULL;
	}
	Array_QuickSort(0, pstArray->iCurrentNum - 1, pstArray->ppvInstanceTable, pstArray->pfObject_Compare);
	return ARRAY_OK;
}

/* write to file */
int Array_Save(Array* pstArray, const char* fileName)
{
	if (pstArray == NULL || pstArray->ppvInstanceTable == NULL) {
		ARRAY_MESSAGE("pstArray or ppvInstanceTable is null");
		return ARRAY_NULL;
	}
	if (fileName == NULL) {
		ARRAY_MESSAGE("fileName is null");
		return ARRAY_NULL;
	}
	if (pstArray->pfObject_Write == NULL) {
		ARRAY_MESSAGE("pfObject_Write is not registered");
		return ARRAY_NULL;
	}
	int i;
	char acBuffer[32] = {0};
	FILE *pstFile = NULL;
	pstFile = fopen(fileName, "w");
	if (pstFile == NULL) {
		ARRAY_MESSAGE("pstFile is NULL ");
		return ARRAY_NULL;
	}
	/* write count */
	sprintf(acBuffer, "%d\n", pstArray->iCurrentNum);
	fputs(acBuffer, pstFile);
	/* write data */
	for (i = 0; i < pstArray->iCurrentNum; i++) {
		pstArray->pfObject_Write(pstFile, pstArray->ppvInstanceTable[i]);
	}
	fclose(pstFile);
	return ARRAY_OK;
}

/* read from file */
int Array_Load(Array* pstArray, const char* fileName)
{
	if (pstArray == NULL || pstArray->ppvInstanceTable == NULL) {
		ARRAY_MESSAGE("pstArray or ppvInstanceTable is null");
		return ARRAY_NULL;
	}
	if (fileName == NULL) {
		ARRAY_MESSAGE("fileName is null");
		return ARRAY_NULL;
	}
	if (pstArray->pfObject_Read == NULL) {
		ARRAY_MESSAGE("pfObject_Read is not registered");
		return ARRAY_NULL;
	}
	if (pstArray->pfObject_New == NULL) {
		ARRAY_MESSAGE("pfObject_New is not registered");
		return ARRAY_NULL;
	}
	int i = 0;
	int count = 0;
	char acBuffer[32] = {0};
	FILE *pstFile = NULL;
	pstFile = fopen(fileName, "r+");
	if (pstFile == NULL) {
		ARRAY_MESSAGE("pstFile is NULL ");
		return ARRAY_NULL;
	}
	/* read count */
	fgets(acBuffer, 32, pstFile);
	count = atoi(acBuffer);
	/* read data */
	for (i = 0; i < count; i++) {
		void* pvInstance = NULL;
		pvInstance = pstArray->pfObject_New();
		if (pvInstance == NULL) {
			continue;
		}
		pstArray->pfObject_Read(pstFile, pvInstance);
		(void)Array_PushBack(pstArray, pvInstance);
	}
	fclose(pstFile);
	return ARRAY_OK;
}
