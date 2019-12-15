#include "list.h"
ListInterface g_stListInterface = {
    List_New,
    List_Delete,
    List_RegisterObject,
    List_Search,
    List_GetFront,
    List_PushBack,
    List_PopBack,
    List_PushFront,
    List_PopFront,
    List_InsertByIndex,
    List_DeleteByIndex,
    List_InsertByValue,
    List_DeleteByValue,
    List_Sort,
    List_Reverse,
    List_Traverse,
    List_Save,
    List_Load
};
Node* List_NewNode(void* pvInstance)
{
	Node *pstNode = NULL;
	pstNode = (Node*)malloc(sizeof(Node));
	if (pstNode == NULL) {
		LIST_MESSAGE("alloc failed");
		return NULL;
	}
	pstNode->pvInstance = pvInstance;
	pstNode->pstNext = NULL;
	pstNode->pstPre = NULL;
	return pstNode;
}

void List_DeleteNode(Node* pstNode, int (*pfObject_Delete)(void* pvInstance))
{
	if (pfObject_Delete != NULL) {
		pfObject_Delete(pstNode->pvInstance);
	}
	free(pstNode);
	return;
}
/* new */
List* List_New()
{
	List *pstList = NULL;
	pstList = (List*)malloc(sizeof(List));
	if (pstList == NULL) {
		LIST_MESSAGE("List alloc failed");
		return NULL;
	}
	pstList->pstHead = NULL;
	pstList->pstTail = NULL;
	pstList->iNodeNum = 0;
	return pstList;
}

/* register object */
int List_RegisterObject(List* pstList,
		void* (*pfObject_New)(),
	    int (*pfObject_Delete)(void* pvInstance),
	    int (*pfObject_Compare)(void* pvInstance1, void* pvInstance2),
	    int (*pfObject_Write)(FILE* pstFile, void* pvInstance),
	    int (*pfObject_Read)(FILE* pstFile, void* pvInstance))
{
	if (pstList == NULL) {
		LIST_MESSAGE("empty pointer");
		return LIST_NULL;
	}
	pstList->pfObject_New = pfObject_New;
	pstList->pfObject_Delete = pfObject_Delete;
	pstList->pfObject_Compare = pfObject_Compare;
	pstList->pfObject_Write = pfObject_Write;
	pstList->pfObject_Read = pfObject_Read;
	return LIST_OK;
}
/* delete */
int List_Delete(List *pstList) 
{
	if (pstList == NULL || pstList->pstHead == NULL) {
		LIST_MESSAGE("empty pointer");
		return LIST_NULL;
	}
	Node *pstNode = pstList->pstHead;
	Node *pstTmp = NULL;
	while (pstNode != NULL) {
		pstTmp = pstNode->pstNext;
		List_DeleteNode(pstNode, pstList->pfObject_Delete);
		pstNode = pstTmp;
	}
	pstList->pstHead = NULL;
	pstList->pstTail = NULL;
	free(pstList);
	return LIST_OK;
}

/* search */
Node *List_Search(const List *pstList, void *pvInstance)
{
	if (pstList == NULL || pstList->pstHead == NULL || pvInstance == NULL) {
		LIST_MESSAGE("empty pointer");
		return NULL;
	}
	if (pstList->pfObject_Compare == NULL) {
		LIST_MESSAGE("pfObject_Compare function is not registered");
		return NULL;
	}
	Node *pstNode = pstList->pstHead;
	while (pstNode != NULL) {
		if (pstList->pfObject_Compare(pstNode->pvInstance, pvInstance) == 0) {
			break;
		}
		pstNode = pstNode->pstNext;
	}
	return pstNode;
}

/* get */
Node* List_GetFront(const List pstList)
{
	if (pstList == NULL || pstList->pstHead == NULL) {
		LIST_MESSAGE("empty pointer");
		return NULL;
	}
    Node* pstNode = pstList->pstHead;
    pstList->pstHead = pstNode->pstNext;
    pstList->pstHead->pstPre = NULL;
    pstNode->pstNext = NULL;
	pstList->iNodeNum--;
    return pstNode;
}

/* push back */
int List_PushBack(List *pstList, void* pvInstance)
{
	if (pstList == NULL || pvInstance == NULL) {
		LIST_MESSAGE("empty pointer");
		return LIST_NULL;
	}
	Node *pstNode = NULL;
	pstNode = List_NewNode(pvInstance);
	if (pstNode == NULL) {
		LIST_MESSAGE("alloc failed");
		return LIST_MEM_ERR;
	}
	if (pstList->iNodeNum == 0) {
		pstList->pstHead = pstNode;
		pstList->pstTail = pstList->pstHead;
	} else {
		pstList->pstTail->pstNext = pstNode;
		pstNode->pstPre = pstList->pstTail;
		pstList->pstTail = pstNode;
	}
	pstList->iNodeNum++;
	return LIST_OK;
}

/* pop back */
int List_PopBack(List *pstList)
{
	if (pstList == NULL || pstList->pstTail == NULL) {
		LIST_MESSAGE("empty pointer");
		return LIST_NULL;
	}
	if (pstList->iNodeNum == 0) {
		LIST_MESSAGE("size error");
		return LIST_SIZE_ERR;
	}
	Node *pstNode = pstList->pstTail;
	pstList->pstTail = pstNode->pstPre;
	pstList->pstTail->pstNext = NULL;
	pstList->iNodeNum--;
	List_DeleteNode(pstNode, pstList->pfObject_Delete);
	return LIST_OK;
}

/* push front */
int List_PushFront(List *pstList, void* pvInstance)
{
	if (pstList == NULL || pvInstance == NULL) {
		LIST_MESSAGE("empty pointer");
		return LIST_NULL;
	}
	Node *pstNode = NULL;
	pstNode = List_NewNode(pvInstance);
	if (pstNode == NULL) {
		LIST_MESSAGE("alloc failed");
		return LIST_MEM_ERR;
	}
	if (pstList->iNodeNum == 0) {
		pstList->pstHead = pstNode;
		pstList->pstTail = pstList->pstHead;
	} else {
		pstList->pstHead->pstPre = pstNode;
		pstNode->pstNext = pstList->pstHead;
		pstList->pstHead = pstNode;
	}
	pstList->iNodeNum++;
	return LIST_OK;
}

/* pop front */
int List_PopFront(List *pstList)
{
	if (pstList == NULL || pstList->pstHead == NULL) {
		LIST_MESSAGE("empty pointer");
		return LIST_NULL;
	}
	if (pstList->iNodeNum == 0) {
		LIST_MESSAGE("size error");
		return LIST_SIZE_ERR;
	}
	Node *pstNode = pstList->pstHead;
	pstList->pstHead = pstNode->pstNext;
	pstList->pstHead->pstPre = NULL;
	pstList->iNodeNum--;
	List_DeleteNode(pstNode, pstList->pfObject_Delete);
	return LIST_OK;
}

/* insert by index */
int List_InsertByIndex(List *pstList, void* pvInstance, int index)
{
	if (pstList == NULL) {
		LIST_MESSAGE("empty List");
		return LIST_NULL;
	}
	if (pvInstance == NULL) {
		LIST_MESSAGE("pvInstance is null");
		return LIST_NULL;
	}
	if (index > pstList->iNodeNum) {
		LIST_MESSAGE("index is out of border");
		return LIST_SIZE_ERR;
	}
	int i = 0;
	Node *pstNode = NULL;
	Node *pstPreNode = NULL;
	Node *pstNextNode = NULL; 
	pstNode = List_NewNode(pvInstance);
	if (pstNode == NULL) {
		LIST_MESSAGE("alloc failed");
		return LIST_MEM_ERR;
	}
	if (pstList->iNodeNum != 0) {
		if (index == 0) {
			pstNextNode = pstList->pstHead->pstNext;
			pstList->pstHead = pstNode;
			pstNode->pstNext = pstNextNode;
		} else if (index == pstList->iNodeNum) {
			pstPreNode = pstList->pstTail;
			pstPreNode->pstNext = pstNode;
			pstNode->pstPre = pstPreNode;
			pstList->pstTail = pstNode;
		} else {
			pstPreNode = pstList->pstHead;
			for (i = 0; i < index; i++) {
				pstPreNode = pstPreNode->pstNext;
			}
			pstNextNode = pstPreNode->pstNext;
			pstPreNode->pstNext = pstNode;
			pstNode->pstPre = pstPreNode;
			pstNode->pstNext = pstNextNode;
			pstNextNode->pstPre = pstNode;
		}
	} else {
		pstList->pstHead = pstNode;
		pstList->pstTail = pstList->pstHead;
	}
	pstList->iNodeNum++;
	return LIST_OK;
}

/* delete by index */
int List_DeleteByIndex(List *pstList, int index)
{
	if (pstList == NULL || pstList->pstHead == NULL) {
		LIST_MESSAGE("empty pointer");
		return LIST_NULL;
	}
	if (pstList->iNodeNum == 0) {
		LIST_MESSAGE("0 pvInstance");
		return LIST_SIZE_ERR;
	}
	if (index >= pstList->iNodeNum) {
		LIST_MESSAGE("index is out of border");
		return LIST_ERR;
	}
	int i = 0;
	Node *pstNode = pstList->pstHead;
	Node *pstPreNode = NULL;
	Node *pstNextNode = NULL; 
	if (index == 0) {
		pstList->pstHead = pstNode->pstNext;
		pstList->pstHead->pstPre = NULL;
	} else if (index == pstList->iNodeNum - 1) {
		pstNode = pstList->pstTail;
		pstList->pstTail = pstNode->pstPre;
		pstList->pstTail->pstNext = NULL;
	} else {
		for (i = 0; i < index; i++) {
			pstNode = pstNode->pstNext;
		}
		pstNextNode = pstNode->pstNext;
		pstPreNode = pstNode->pstPre;
		pstPreNode->pstNext = pstNextNode;
		pstNextNode->pstPre = pstPreNode;
	}
	pstList->iNodeNum--;
	List_DeleteNode(pstNode, pstList->pfObject_Delete);
	return LIST_OK;
}

/* insert by value */
int List_InsertByValue(List *pstList, void* pvPreInstance, void* pvInstance)
{
	if (pstList == NULL || pstList->pstHead == NULL || pvInstance == NULL) {
		LIST_MESSAGE("empty pointer");
		return LIST_NULL;
	}
	if (pstList->iNodeNum == 0) {
		LIST_MESSAGE("0 pvInstance");
		return LIST_SIZE_ERR;
	}
	Node *pstNode = NULL;
	Node *pstPreNode = NULL;
	Node *pstNextNode = NULL; 
	pstNode = List_NewNode(pvInstance);
	if (pstNode == NULL) {
		LIST_MESSAGE("alloc failed");
		return LIST_MEM_ERR;
	}
	pstPreNode = List_Search(pstList, pvPreInstance);
	if (pstPreNode == NULL) {
		LIST_MESSAGE("pvInstance not found");
		return LIST_NULL;
	}
	pstNextNode = pstPreNode->pstNext;
	pstPreNode->pstNext = pstNode;
	pstNode->pstPre = pstPreNode;
	pstNode->pstNext = pstNextNode;
	if (pstNextNode != NULL) {
		pstNextNode->pstPre = pstNode;
	} else {
		pstList->pstTail = pstNode;
	}
	pstList->iNodeNum++;
	return LIST_OK;
}

/* delete by value */
int List_DeleteByValue(List *pstList, void* pvInstance)
{
	if (pstList == NULL || pstList->pstHead == NULL || pvInstance == NULL) {
		LIST_MESSAGE("empty pointer");
		return LIST_NULL;
	}
	if (pstList->iNodeNum == 0) {
		LIST_MESSAGE("0 pvInstance");
		return LIST_SIZE_ERR;
	}
	Node *pstNode = NULL;
	Node *pstPreNode = NULL;
	Node *pstNextNode = NULL; 
	pstNode = List_Search(pstList, pvInstance);
	if (pstNode == NULL) {
		LIST_MESSAGE("pvInstance is not found");
		return LIST_NULL;
	}
	pstNextNode = pstNode->pstNext;
	pstPreNode = pstNode->pstPre;
	pstPreNode->pstNext = pstNextNode;
	pstNextNode->pstPre = pstPreNode;
	pstList->iNodeNum--;
	List_DeleteNode(pstNode, pstList->pfObject_Delete);
	return LIST_OK;
}

static void List_BubbleSort(List *pstList)
{
	Node *pstNode1 = NULL;
	Node *pstNode2 = NULL;
	void* pvTmp = NULL;
	pstNode1 = pstList->pstHead;
	while (pstNode1 != NULL) {
		pstNode2 = pstNode1->pstNext;
		while (pstNode2 != NULL) {
			if (pstList->pfObject_Compare(pstNode1->pvInstance, pstNode2->pvInstance) < 0) {
				pvTmp = pstNode1->pvInstance;
				pstNode1->pvInstance = pstNode2->pvInstance;
				pstNode2->pvInstance = pvTmp;
			}
			pstNode2 = pstNode2->pstNext;
		}
		pstNode1 = pstNode1->pstNext;
	}
	return;
}
/* sort */
int List_Sort(List *pstList)
{
	if (pstList == NULL || pstList->pstHead == NULL || pstList->pstTail == NULL) {
		LIST_MESSAGE("empty pointer");
		return LIST_NULL;
	}
	if (pstList->iNodeNum <= 1) {
		LIST_MESSAGE("0 or 1 pvInstance");
		return LIST_SIZE_ERR;
	}
	if (pstList->pfObject_Compare == NULL) {
		LIST_MESSAGE("pfObject_Compare is not registered");
		return LIST_NULL;
	}
	List_BubbleSort(pstList);
	return LIST_OK;
}

/* reverse */
int List_Reverse(List *pstList)
{
	if (pstList == NULL || pstList->pstHead == NULL || pstList->pstTail == NULL) {
		LIST_MESSAGE("empty pointer");
		return LIST_NULL;
	}
	if (pstList->iNodeNum <= 1) {
		LIST_MESSAGE("0 or 1 pvInstance");
		return LIST_SIZE_ERR;
	}
	pstList->pstTail = pstList->pstHead;
	Node *pstNode0 = NULL;
	Node *pstNode1 = pstList->pstHead;
	Node *pstNode2 = NULL;
	while (pstNode1 != NULL) {
		pstNode2 = pstNode1->pstNext;
		pstNode1->pstNext = pstNode0;
		pstNode1->pstPre = pstNode2;
		pstNode0 = pstNode1;
		pstNode1 = pstNode2;
	}
	pstList->pstHead = pstNode0;
	return LIST_OK;
}

/* traverse */
int List_Traverse(const List *pstList, int (*pfCallBack)(void* pvInstance, void* pvArg), void* pvArg)
{
	if (pstList == NULL || pstList->pstHead == NULL) {
		LIST_MESSAGE("empty pointer");
		return LIST_NULL;
	}
	if (pfCallBack == NULL) {
		LIST_MESSAGE("call back function is not register");
		return LIST_NULL;
	}
	Node *pstNode = pstList->pstHead;
	while (pstNode != NULL) {
		pfCallBack(pstNode->pvInstance, pvArg);
		pstNode = pstNode->pstNext;
	}
	return LIST_OK;
}

/* write to file */
int List_Save(List *pstList, const char* pcFileName)
{
	if (pstList == NULL || pstList->pstHead == NULL) {
		LIST_MESSAGE("pstList or pstList pstHead is null");
		return LIST_NULL;
	}
	if (pcFileName == NULL) {
		LIST_MESSAGE("pcFileName is null");
		return LIST_NULL;
	}
	if (pstList->pfObject_Write == NULL) {
		LIST_MESSAGE("pfObject_Write is null");
		return LIST_NULL;
	}
	char acBuffer[32] = {0};
	FILE *pstFile = NULL;
	pstFile = fopen(pcFileName, "w");
	if (pstFile == NULL) {
		LIST_MESSAGE("open file failed");
		return LIST_NULL;
	}
	/* write number of instance */
	sprintf(acBuffer, "%d\n", pstList->iNodeNum);
	fputs(acBuffer, pstFile);
	/* write instance */
	Node *pstNode = pstList->pstHead;
	while (pstNode != NULL) {
		pstList->pfObject_Write(pstFile, pstNode->pvInstance);
		pstNode = pstNode->pstNext;
	}
	fclose(pstFile);
	return LIST_OK;
}

/* read from file */
int List_Load(List *pstList, const char* pcFileName)
{
	if (pstList == NULL) {
		LIST_MESSAGE("pstList or pstList pstHead is null");
		return LIST_NULL;
	}
	if (pcFileName == NULL) {
		LIST_MESSAGE("pcFileName is null");
		return LIST_NULL;
	}
	if (pstList->pfObject_Read == NULL) {
		LIST_MESSAGE("pfObject_Read is null");
		return LIST_NULL;
	}
	if (pstList->pfObject_New == NULL) {
		LIST_MESSAGE("pfObject_New is null");
		return LIST_NULL;
	}
	int i;
	int iInstanceNum = 0;
	char acBuffer[32] = {0};
	FILE *pstFile = NULL;
	pstFile = fopen(pcFileName, "r");
	if (pstFile == NULL) {
		LIST_MESSAGE("open file failed");
		return LIST_NO_FILE;
	}
	/* read number of instance */
	fgets(acBuffer, 32, pstFile);
	iInstanceNum = atoi(acBuffer);
	if (iInstanceNum < 1) {
		fclose(pstFile);
		return LIST_SIZE_ERR;
	}
	/* read instance */
	for (i = 0; i < iInstanceNum; i++) {
		void* pvInstance = NULL;
		pvInstance = pstList->pfObject_New();
		if (pvInstance == NULL) {
			continue;
		}
		pstList->pfObject_Read(pstFile, pvInstance);
		(void)List_PushBack(pstList, pvInstance);
	}
	fclose(pstFile);
	return LIST_OK;
}
