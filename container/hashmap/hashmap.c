#include "hashmap.h"

HashMapInterface g_stHashMapInterface;
    HashMap_Create,
    HashMap_Destroy,
    HashMap_RegisterObject,
    HashMap_Insert,
    HashMap_Delete,
    HashMap_Search,
    HashMap_Traverse,
    HashMap_Resize,
    HashMap_Load,
    HashMap_Save
};

HashNode* HashMap_CreateNode(const char* pcKey, void* pvInstance)
{
	HashNode* pstNode = NULL;
	pstNode = (HashNode *)malloc(sizeof(HashNode));
	if (pstNode == NULL) {
		HASHMAP_MESSAGE("fail to malloc hash node");
		return NULL;
	}
	pstNode->pcKey = strdup(pcKey);
	pstNode->pvInstance = pvInstance;
	pstNode->pstNext = NULL;
	return pstNode;
}
void HashMap_DestroyNode(HashNode* pstHashNode, int (*pfObject_Delete)(void* pvInstance))
{
	if (pstHashNode == NULL) {
		return;
	}
	if (pstHashNode->pcKey != NULL) {
		free(pstHashNode->pcKey);
	}
	if (pstHashNode->pvInstance != NULL && pfObject_Delete != NULL) {
		pfObject_Delete(pstHashNode->pvInstance);
	}
	free(pstHashNode);
	return;
}


unsigned int HashMap_APHash(const char *str)
{
	unsigned int hash = 0;
	int i;
	for (i=0; *str; i++) {
		if ((i & 1) == 0) {
			hash ^= ((hash << 7) ^ (*str++) ^ (hash >> 3));
		} else {
			hash ^= (~((hash << 11) ^ (*str++) ^ (hash >> 5)));
		}
	}
	return (hash & 0x7FFFFFFF);
}

unsigned int HashMap_Func(const char* pcKey)
{
	return HashMap_APHash(pcKey);
}

HashMap* HashMap_Create(int maxListNum)
{
	if (maxListNum < 1) {
		return NULL;
	}
	HashMap* pstHashMap = NULL;
	do {
		pstHashMap = (HashMap*)malloc(sizeof(HashMap));
		if (pstHashMap == NULL) {
			HASHMAP_MESSAGE("pstHashMap is null");
			break;
		}
		pstHashMap->ppstHashTable = (HashNode**)malloc(sizeof(HashNode*) * maxListNum);
		if (pstHashMap->ppstHashTable == NULL) {
			HASHMAP_MESSAGE("pstHashTable is null");
			break;
		}
		pstHashMap->nodeNum = 0;
		pstHashMap->maxListNum = maxListNum;
		pstHashMap->pfObject_New = NULL;
		pstHashMap->pfObject_Delete = NULL;
		pstHashMap->pfObject_Write = NULL;
		pstHashMap->pfObject_Read = NULL;
		int i = 0;
		for (i = 0; i < maxListNum; i++) {
			pstHashMap->ppstHashTable[i] = NULL;
		}
		return pstHashMap;
	} while (0);
	HashMap_Destroy(pstHashMap);
	return NULL;
}

int HashMap_Destroy(HashMap* pstHashMap)
{
	if (pstHashMap == NULL) {
		return;
	}
	if (pstHashMap->ppstHashTable != NULL) {
		int i = 0;
		HashNode* *ppstTable = pstHashMap->ppstHashTable;
		for (i = 0; i < pstHashMap->maxListNum; i++) {
			HashNode* pstHead = ppstTable[i];
			HashNode* pstTmp = NULL;
			while (pstHead != NULL) {
				pstTmp = pstHead;
				pstHead = pstHead->pstNext;
				HashMap_DestroyNode(pstTmp, pstHashMap->pfObject_Delete);
			}
		}
		free(pstHashMap->ppstHashTable);
	}
	free(pstHashMap);
	return;
}
int HashMap_RegisterObject(
		HashMap* pstHashMap,
		void* (*pfObject_New)(),
		int (*pfObject_Delete)(void* pvInstance),
	    int (*pfObject_Compare)(void* pvInstance1, void* pvInstance2),
		int (*pfObject_Write)(FILE* pstFile, void* pvInstance),
		int (*pfObject_Read)(FILE* pstFile, void* pvInstance))
{
	if (pstHashMap == NULL) {
		return HASHMAP_NULL; 
	}
	pstHashMap->pfObject_New = pfObject_New;
	pstHashMap->pfObject_Delete = pfObject_Delete;
	pstHashMap->pfObject_Compare = pfObject_Compare;
	pstHashMap->pfObject_Write = pfObject_Write;
	pstHashMap->pfObject_Read = pfObject_Read;
	return HASHMAP_OK;
}

int HashMap_Insert(HashMap* pstHashMap, const char* pcKey, void* pvInstance)
{
	if (pstHashMap == NULL || pcKey == NULL || pvInstance == NULL) {
		HASHMAP_MESSAGE("pointer is null");
		return HASHMAP_NULL;
	}
	if (pstHashMap->ppstHashTable == NULL) {
		HASHMAP_MESSAGE("pointer is null");
		return HASHMAP_NULL;
	}
	HashNode *pstHashNode = NULL;
	pstHashNode = HashMap_CreateNode(pcKey, pvInstance);
	if (pstHashNode == NULL) {
		HASHMAP_MESSAGE("pstHashNode is null");
		return HASHMAP_MEM_ERR;
	}
	HashNode *pstNode = NULL;
	unsigned int index = HashMap_Func(pcKey) % pstHashMap->maxListNum;	
	printf("index = %u\n", index);
	pstNode = pstHashMap->ppstHashTable[index];
	if (pstNode != NULL) {
		pstHashNode->pstNext = pstNode;
	}
	pstHashMap->ppstHashTable[index] = pstHashNode;
	pstHashMap->nodeNum++;
	return HASHMAP_OK;
}

int HashMap_Delete(HashMap* pstHashMap, const char* pcKey)
{
	if (pstHashMap == NULL || pcKey == NULL) {
		return HASHMAP_NULL;
	}
	if (pstHashMap->ppstHashTable == NULL) {
		return HASHMAP_NULL;
	}
	unsigned int index;
	HashNode *pstNode = NULL;
	HashNode *pstPre = NULL;
	index = HashMap_Func(pcKey) % pstHashMap->maxListNum;
	pstNode = pstHashMap->ppstHashTable[index];
	if (pstNode == NULL) {
		return HASHMAP_NOT_FOUND;
	}
	while (pstNode != NULL) {
		if (strcmp(pstNode->pcKey, pcKey) == 0) {
			break;
		}
		pstPre = pstNode;
		pstNode = pstNode->pstNext;
	}
	if (pstPre == NULL) {
		pstHashMap->ppstHashTable[index] = pstNode->pstNext;
	} else {
		pstPre->pstNext = pstNode->pstNext;
	}
	HashMap_DestroyNode(pstNode, pstHashMap->pfObject_Delete);
	pstHashMap->nodeNum--;
	return HASHMAP_OK;
}

int HashMap_Search(HashMap* pstHashMap, const char* pcKey, void** ppvInstance)
{
	if (pstHashMap == NULL || pcKey == NULL || ppvInstance == NULL) {
		return HASHMAP_NULL;
	}
	if (pstHashMap->ppstHashTable == NULL) {
		return HASHMAP_NULL;
	}
	unsigned int index;
	index = HashMap_Func(pcKey) % pstHashMap->maxListNum;
	HashNode* pstNode = NULL;
	pstNode = pstHashMap->ppstHashTable[index];
	while (pstNode != NULL) {
		if (strcmp(pstNode->pcKey, pcKey) == 0) {
			*ppvInstance = pstNode->pvInstance;
			return HASHMAP_OK;
		}
		pstNode = pstNode->pstNext;
	}
	return HASHMAP_NULL;
}
int HashMap_Traverse(HashMap* pstHashMap, int (*pfCallBack)(HashNode* pstNode, void* pArg), void* pArg)
{
	if (pstHashMap == NULL || pfCallBack == NULL) {
		HASHMAP_MESSAGE("pstHashMap is null");
		return HASHMAP_NULL;
	}
	if (pstHashMap->ppstHashTable == NULL) {
		HASHMAP_MESSAGE("pstHashtable is null");
		return HASHMAP_NULL;
	}
	int i;
	for (i = 0; i < pstHashMap->maxListNum; i++) {
		HashNode* pstNode = pstHashMap->ppstHashTable[i];
		while (pstNode != NULL) {
			pfCallBack(pstNode, pArg);
			pstNode = pstNode->pstNext;
		}
	}
	return HASHMAP_OK;
}

int HashMap_Resize(HashMap* pstHashMap, int iNewSize)
{
	if (pstHashMap == NULL) {
		return HASHMAP_NULL;
	}
	if (pstHashMap->ppstHashTable == NULL) {
		return HASHMAP_NULL;
	}
	if (pstHashMap->maxListNum >= iNewSize) {
		return HASHMAP_SIZE_ERR;
	}
	HashNode* *ppstNewTable = NULL;
	ppstNewTable = (HashNode**)malloc(sizeof(HashNode*) * iNewSize);
	if (ppstNewTable == NULL) {
		return HASHMAP_MEM_ERR;
	}
	int i;
	for (i = 0; i < iNewSize; i++) {
		ppstNewTable[i] = NULL;
	}

	for (i = 0; i < pstHashMap->maxListNum; i++) {
		HashNode* pstNode = NULL;
	    HashNode* pstNext = NULL;
		pstNode = pstHashMap->ppstHashTable[i];
		while (pstNode != NULL) {
			pstNext = pstNode->pstNext;
			pstNode->pstNext = NULL;
			unsigned int index = HashMap_Func(pstNode->pcKey) % iNewSize;
			printf("index = %u\n", index);
			if (ppstNewTable[index] != NULL) {
				pstNode->pstNext = ppstNewTable[index];
			}
			ppstNewTable[index] = pstNode;
			pstNode = pstNext;
		}
	}
	free(pstHashMap->ppstHashTable);
	pstHashMap->ppstHashTable = ppstNewTable;
	pstHashMap->maxListNum = iNewSize;
	return HASHMAP_OK;
}

int HashMap_Load(HashMap* pstHashMap, const char* pcFileName)
{
	if (pstHashMap == NULL || pcFileName == NULL) {
		HASHMAP_MESSAGE("hashmap is null or file name is null");
		return HASHMAP_NULL;
	}
	if (pstHashMap->ppstHashTable == NULL) {
		HASHMAP_MESSAGE("hashtable is null");
		return HASHMAP_NULL;
	}
	if (pstHashMap->pfObject_New == NULL) {
		HASHMAP_MESSAGE("Object_New is not registered");
		return HASHMAP_NULL;
	}
	if (pstHashMap->pfObject_Read == NULL) {
		HASHMAP_MESSAGE("Object_Read is not registered");
		return HASHMAP_NULL;
	}
	int i;
	int iObjectNum = 0;
	char acBuffer[HASHMAP_BUFFER_LEN] ={0};
	char acKey[HASHMAP_BUFFER_LEN] ={0};
	FILE* pstFile = fopen(pcFileName, "r");
	if (pstFile == NULL) {
		return HASHMAP_NO_FILE;
	}
	/* read number of object  */
	fgets(acBuffer, HASHMAP_BUFFER_LEN, pstFile);
	iObjectNum = atoi(acBuffer);
	/* read data */
	for (i = 0; i < iObjectNum; i++) {
		/* read key */
		memset(acKey, 0, HASHMAP_BUFFER_LEN);
		fgets(acKey, HASHMAP_BUFFER_LEN, pstFile);
		if (strlen(acKey) < 1) {
			strcpy(acKey, "unknow");
		}
		/* read object */
		void* pvInstance = NULL;
		pvInstance = pstHashMap->pfObject_New();
		if (pvInstance == NULL) {
			continue;
		}
		pstHashMap->pfObject_Read(pstFile, pvInstance);
		/* insert */
		(void)HashMap_Insert(pstHashMap, acKey, pvInstance);
	}
	fclose(pstFile);
	return HASHMAP_OK;
}

int HashMap_Save(HashMap* pstHashMap, const char* pcFileName)
{
	if (pstHashMap == NULL || pcFileName == NULL) {
		HASHMAP_MESSAGE("hashmap is null or file name is null");
		return HASHMAP_NULL;
	}
	if (pstHashMap->ppstHashTable == NULL) {
		HASHMAP_MESSAGE("hashtable is null");
		return HASHMAP_NULL;
	}
	if (pstHashMap->pfObject_Write == NULL) {
		HASHMAP_MESSAGE("Object_Write is not registered");
		return HASHMAP_NULL;
	}
	int i;
	char acBuffer[HASHMAP_BUFFER_LEN] ={0};
	FILE* pstFile = fopen(pcFileName, "w");
	if (pstFile == NULL) {
		return HASHMAP_NO_FILE;
	}
	/* write number of object  */
	memset(acBuffer, 0, HASHMAP_BUFFER_LEN);
	sprintf(acBuffer, "%d\n", pstHashMap->nodeNum);
	fputs(acBuffer, pstFile);
	/* write data */
	for (i = 0; i < pstHashMap->maxListNum; i++) {
		HashNode* pstNode = pstHashMap->ppstHashTable[i];
		while (pstNode != NULL) {
			/* write key */
			memset(acBuffer, 0, HASHMAP_BUFFER_LEN);
			sprintf(acBuffer, "%s\n", pstNode->pcKey);
			fputs(acBuffer, pstFile);
			/* write object */
			pstHashMap->pfObject_Write(pstNode->pvInstance, pstFile);
			/* next */
			pstNode = pstNode->pstNext;
		}
	}
	fclose(pstFile);
	return HASHMAP_OK;
}
