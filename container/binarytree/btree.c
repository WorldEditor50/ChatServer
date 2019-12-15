#include "btree.h"

BTreeInterface g_stBTreeInterface = {
    BTree_New,
    BTree_RegisterObject,
    BTree_Delete,
    BTree_Insert,
    BTree_Remove,
    BTree_Traverse,
    BTree_Search,
    BTree_Save,
    BTree_Load
};

TriNode* TriNode_New(void* pvInstance)
{
	if (pvInstance == NULL) {
		BTREE_MESSAGE("pvInstance is null");
		return NULL;
	}
	TriNode* pstNode = NULL;
	pstNode = (TriNode*)malloc(sizeof(TriNode));
	if (pstNode == NULL) {
		BTREE_MESSAGE("Node malloc failed");
		return NULL;
	}
	pstNode->pvInstance = pvInstance;
	pstNode->pstUp = NULL;
	pstNode->pstLeft = NULL;
	pstNode->pstRight = NULL;
	return pstNode;
}
/* New */
BTree* BTree_New(int (*pfObject_Compare)(void* pvInstance1, void* pvInstance2))
{
	if (pfObject_Compare == NULL) {
		BTREE_MESSAGE("pfObject_Compare is null");
		return NULL;
	}
	BTree* pstBTree = NULL;
	pstBTree = (BTree*)malloc(sizeof(BTree));
	if (pstBTree == NULL) {
		BTREE_MESSAGE("tree malloc failed");
		return NULL;
	}
	pstBTree->pstRoot = NULL;
	pstBTree->iNodeNum = 0;
	pstBTree->pfObject_New = NULL;
	pstBTree->pfObject_Delete = NULL;
	pstBTree->pfObject_Write = NULL;
	pstBTree->pfObject_Read = NULL;
	pstBTree->pfObject_Compare = pfObject_Compare;
	return pstBTree;
}

/* register object */
int BTree_RegisterObject(BTree* pstBTree,
		void* (*pfObject_New)(),
	    int (*pfObject_Delete)(void* pvInstance),
		int (*pfObject_Compare)(void* pvInstance1, void* pvInstance2),
	    int (*pfObject_Write)(FILE* pstFile, void* pvInstance),
	    int (*pfObject_Read)(FILE* pstFile, void* pvInstance))
{
	if (pstBTree == NULL) {
		return BTREE_NULL;
	}
	pstBTree->pfObject_New = pfObject_New;
	pstBTree->pfObject_Delete = pfObject_Delete;
	pstBTree->pfObject_Compare = pfObject_Compare;
	pstBTree->pfObject_Write = pfObject_Write;
	pstBTree->pfObject_Read = pfObject_Read;
	return BTREE_OK;
}

/* free */
void BTree_Free(TriNode** ppstRoot, int (*pfObject_Delete)(void* pvInstance))
{
	if (*ppstRoot == NULL) {
		return;
	}
	TriNode* pstNode = *ppstRoot;
	TriNode* pstLeft = pstNode->pstLeft;
	TriNode* pstRight = pstNode->pstRight;
	BTree_Free(&pstLeft, pfObject_Delete);
	BTree_Free(&pstRight, pfObject_Delete);
	pfObject_Delete(pstNode->pvInstance);
	pstNode->pvInstance = NULL;
	free(pstNode);
	*ppstRoot = NULL;
	return;
}
/* delete */
int BTree_Delete(BTree* pstBTree)
{
	if (pstBTree == NULL || pstBTree->pstRoot == NULL) {
		BTREE_MESSAGE("pstBTree or pstRoot is null");
		return BTREE_NULL;
	}
	if (pstBTree->pfObject_Delete == NULL) {
		BTREE_MESSAGE("pfObject_Delete is not registered");
		return BTREE_NULL;
	}
	BTree_Free(&pstBTree->pstRoot, pstBTree->pfObject_Delete);
	pstBTree->pstRoot = NULL;
	free(pstBTree);
	return BTREE_OK;
}

TriNode* BTree_LookUp(TriNode* pstRoot, int (*pfObject_Compare)(void* pvInstance1, void* pvInstance2), void* pvInstance)
{
	TriNode* pstNode = pstRoot;
	int iFlag = 1;
	while (pstNode != NULL) {
		iFlag = pfObject_Compare(pvInstance, pstNode->pvInstance);
		if (iFlag > 0) {
			pstNode = pstNode->pstLeft;
		} else if (iFlag < 0) {
			pstNode = pstNode->pstRight;
		} else {
			break;
		}
	}
	return pstNode;
}
/* search */
int BTree_Search(BTree* pstBTree, void* pvInstance)
{
	if (pstBTree == NULL || pstBTree->pstRoot == NULL) {
		BTREE_MESSAGE("tree is null");
		return BTREE_NULL;
	}
	if (pstBTree->pfObject_Compare == NULL) {
		BTREE_MESSAGE("pfObject_Compare is not registered");
		return BTREE_NULL;
	}
	if (pvInstance == NULL) {
		BTREE_MESSAGE("pvInstance is null");
		return BTREE_NULL;
	}
	TriNode* pstNode = NULL;
	pstNode = BTree_LookUp(pstBTree->pstRoot, pstBTree->pfObject_Compare, pvInstance);
	if (pstNode == NULL) {
		return BTREE_NOT_FOUND;
	}
	return BTREE_OK;
}
/* insert */
void BTree_InsertNode(TriNode* pstRoot, TriNode* pstNode, int (*pfObject_Compare)(void* pvInstance1, void* pvInstance2))
{
	TriNode* pstNextNode = pstRoot;	
	int iFlag = 1;
	while (pstNextNode != NULL) {
		iFlag = pfObject_Compare(pstNode->pvInstance, pstNextNode->pvInstance);
		if (iFlag >= 0) {
			if (pstNextNode->pstLeft != NULL) {
				pstNextNode = pstNextNode->pstLeft;
			} else {
				pstNextNode->pstLeft = pstNode;
				pstNode->pstUp = pstNextNode;
				break;
			} 
		} else {
			if (pstNextNode->pstRight != NULL) {
				pstNextNode = pstNextNode->pstRight;
			} else {
				pstNextNode->pstRight = pstNode;
				pstNode->pstUp = pstNextNode;
				break;
			} 
		}
	}
	return;
}
int BTree_Insert(BTree* pstBTree, void* pvInstance)
{
	if (pstBTree == NULL) {
		BTREE_MESSAGE("tree is null");
		return BTREE_NULL;
	}
	if (pstBTree->pfObject_Compare == NULL) {
		BTREE_MESSAGE("pfObject_Compare is not registered");
		return BTREE_NULL;
	}
	if (pvInstance == NULL) {
		BTREE_MESSAGE("pvInstance is null");
		return BTREE_NULL;
	}
	TriNode* pstNode = NULL;
	pstNode = TriNode_New(pvInstance);
	if (pstNode == NULL) {
		BTREE_MESSAGE("New node failed");
		return BTREE_MEM_ERR;
	}
	if (pstBTree->pstRoot == NULL) {
		pstBTree->pstRoot = pstNode;
	} else {
		BTree_InsertNode(pstBTree->pstRoot, pstNode, pstBTree->pfObject_Compare);
	}
	pstBTree->iNodeNum++;
	return BTREE_OK;
}

int BTree_Remove(BTree* pstBTree, void* pvInstance)
{
	if (pstBTree == NULL || pstBTree->pstRoot == NULL) {
		BTREE_MESSAGE("tree is null");
		return BTREE_NULL;
	}
	if (pstBTree->pfObject_Compare == NULL) {
		BTREE_MESSAGE("pfObject_Compare is not registered");
		return BTREE_NULL;
	}
	if (pvInstance == NULL) {
		BTREE_MESSAGE("pvInstance is null");
		return BTREE_NULL;
	}
	TriNode* pstNode = NULL;
	TriNode* pstLeft = NULL;
	TriNode* pstRight = NULL;
	TriNode* pstUp = NULL;
	TriNode* pstTail = NULL;
	pstNode = BTree_LookUp(pstBTree->pstRoot, pstBTree->pfObject_Compare, pvInstance);
	if (pstNode == NULL) {
		BTREE_MESSAGE("pvInstance is not exist");
		return BTREE_NULL;
	}
	if (pstNode->pstUp == NULL) {
		if (pstNode->pstLeft == NULL && pstNode->pstRight == NULL) {

		} else if (pstNode->pstLeft == NULL && pstNode->pstRight != NULL) {
			pstRight = pstNode->pstRight;
			pstRight->pstUp = NULL;
			pstBTree->pstRoot = pstRight;
		} else if (pstNode->pstLeft != NULL && pstNode->pstRight == NULL) {
			pstLeft = pstNode->pstLeft;
			pstLeft->pstUp = NULL;
			pstBTree->pstRoot = pstLeft;
		} else if (pstNode->pstLeft != NULL && pstNode->pstRight != NULL) {
			pstLeft = pstNode->pstLeft;
			pstRight = pstNode->pstRight;
			pstTail = pstLeft;
			while (pstTail->pstRight != NULL) {
				pstTail = pstTail->pstRight;
			}
			pstTail->pstRight = pstRight;
			pstLeft->pstUp = NULL;
			pstBTree->pstRoot = pstLeft;
		}
	} else {
		if (pstNode->pstLeft == NULL && pstNode->pstRight == NULL) {
			pstUp = pstNode->pstUp;
			if (pstBTree->pfObject_Compare(pvInstance, pstUp->pvInstance) > 0) {
				pstUp->pstLeft = NULL;
			} else {
				pstUp->pstRight = NULL;
			}
		} else if (pstNode->pstLeft == NULL && pstNode->pstRight != NULL) {
			pstUp = pstNode->pstUp;
			pstRight = pstNode->pstRight;
			if (pstBTree->pfObject_Compare(pvInstance, pstUp->pvInstance) > 0) {
				pstUp->pstLeft = pstRight;
			} else {
				pstUp->pstRight = pstRight;
			}
			pstRight->pstUp = pstUp;
		} else if (pstNode->pstLeft != NULL && pstNode->pstRight == NULL) {
			pstUp = pstNode->pstUp;
			pstLeft = pstNode->pstLeft;
			if (pstBTree->pfObject_Compare(pvInstance, pstUp->pvInstance) >= 0) {
				pstUp->pstLeft = pstLeft;
			} else {
				pstUp->pstRight = pstLeft;
			}
			pstLeft->pstUp = pstUp;
		} else if (pstNode->pstLeft != NULL && pstNode->pstRight != NULL) {
			pstUp = pstNode->pstUp;
			pstLeft = pstNode->pstLeft;
			pstRight = pstNode->pstRight;
			pstTail = pstLeft;
			if (pstBTree->pfObject_Compare(pvInstance, pstUp->pvInstance) >= 0) {
				pstUp->pstLeft = pstLeft;
			} else {
				pstUp->pstRight = pstLeft;
			}
			while (pstTail->pstRight != NULL) {
				pstTail = pstTail->pstRight;
			}
			pstTail->pstRight = pstRight;	
			pstLeft->pstUp = pstUp;
			pstRight->pstUp = pstTail;
		}
	}
	if (pstBTree->pfObject_Delete != NULL) {
		pstBTree->pfObject_Delete(pvInstance);
	}
	free(pstNode);
	pstNode = NULL;
	pstBTree->iNodeNum--;
	return BTREE_OK;
}

/* traverse */
void BTree_TraverseByRecurrent(TriNode* pstRoot, int (*pfCallBack)(void* pvInstance, void* pvArg), void* pvArg)
{
	if (pstRoot == NULL) {
		return;
	}
	BTree_TraverseByRecurrent(pstRoot->pstLeft, pfCallBack, pvArg);
	pfCallBack(pstRoot->pvInstance, pvArg);
	BTree_TraverseByRecurrent(pstRoot->pstRight, pfCallBack, pvArg);
	return;
}
int BTree_Traverse(BTree* pstBTree, int (*pfCallBack)(void* pvInstance, void* pvArg), void* pvArg)
{
	if (pstBTree == NULL || pstBTree->pstRoot == NULL) {
		BTREE_MESSAGE("tree is null");
		return BTREE_NULL;
	}
	if (pfCallBack == NULL) {
		BTREE_MESSAGE("callBack is not registered");
		return BTREE_NULL;
	}
	BTree_TraverseByRecurrent(pstBTree->pstRoot, pfCallBack, pvArg);
	return BTREE_OK;
}

/* write to file */
void BTree_Write(TriNode* pstRoot, int (*pfObject_Write)(FILE* pstFile, void* pvInstance), FILE* pstFile)
{
	if (pstRoot == NULL) {
		return;
	}
	BTree_Write(pstRoot->pstLeft, pfObject_Write, pstFile);
	pfObject_Write(pstFile, pstRoot->pvInstance);
	BTree_Write(pstRoot->pstRight, pfObject_Write, pstFile);
	return;
}
int BTree_Save(BTree* pstBTree, const char* pcFileName)
{
	if (pstBTree == NULL || pstBTree->pstRoot == NULL) {
		BTREE_MESSAGE("tree is null");
		return BTREE_NULL;
	}
	if (pstBTree->pfObject_Write == NULL) {
		BTREE_MESSAGE("pfObject_Write is not registered");
		return BTREE_NULL;
	}
	char acBuffer[32] = {0};
	FILE* pstFile = NULL;
	pstFile = fopen(pcFileName, "w");
	if (pstFile == NULL) {
		BTREE_MESSAGE("open file failed");
		return BTREE_NO_FILE;
	}
	sprintf(acBuffer, "%d\n", pstBTree->iNodeNum);
	fputs(acBuffer, pstFile);
	BTree_Write(pstBTree->pstRoot, pstBTree->pfObject_Write, pstFile);
	fclose(pstFile);
	return BTREE_OK;
}

int BTree_Load(BTree* pstBTree, const char* pcFileName)
{
	if (pstBTree == NULL) {
		BTREE_MESSAGE("tree is null");
		return BTREE_NULL;
	}
	if (pstBTree->pfObject_Read == NULL) {
		BTREE_MESSAGE("pfObject_Read is not registered");
		return BTREE_NULL;
	}
	if (pstBTree->pfObject_New == NULL) {
		BTREE_MESSAGE("pfObject_New is not registered");
		return BTREE_NULL;
	}
	int i = 0;
	int iObjectNum = 0;
	char acBuffer[32] = {0};
	FILE* pstFile = NULL;
	pstFile = fopen(pcFileName, "r");
	if (pstFile == NULL) {
		BTREE_MESSAGE("open file failed");
		return BTREE_NO_FILE;
	}
	fgets(acBuffer, 32, pstFile);
	iObjectNum = atoi(acBuffer);
	for (i = 0; i < iObjectNum; i++) {
		void* pvInstance = NULL;
		pvInstance = pstBTree->pfObject_New();
		if (pvInstance == NULL) {
			continue;
		}
		pstBTree->pfObject_Read(pstFile, pvInstance);
		(void)BTree_Insert(pstBTree, pvInstance);
	}
	fclose(pstFile);
	return BTREE_OK;
}
