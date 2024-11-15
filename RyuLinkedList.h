#pragma once

#ifndef DARK_NEW
#define DARK_NEW	new
#endif

typedef struct RyuLinkedListST *pRyuLinkedListST;

struct RyuLinkedListST {
	void *pData;
	pRyuLinkedListST pNext;
};

template <typename T> 
class RyuLinkedList
{
public:
	RyuLinkedList(void) {
		m_pList = 0;
		m_pReadCursor = 0;
	}
	~RyuLinkedList(void) {
		DeleteAll();
	}

#ifndef RYULINK_NO_DEFAULT_TYPE
	T *Add(void) {
		T *pData;

		pData = DARK_NEW T;
		if (pData == 0) return 0;
		Add(pData);

		return pData;
	}
#endif

	void Add(T *pdata) {
		pRyuLinkedListST pT, pT2;

		pT = DARK_NEW RyuLinkedListST;
		if (pT == 0) return;
		pT->pData = pdata;
		pT->pNext = 0;

		if( m_pList == 0 ) m_pList = pT;
		else {
			pT2 = m_pList;
			while(pT2->pNext != 0) pT2 = pT2->pNext;
			pT2->pNext = pT;
		}
	}

	T *Insert(T *pTo) {
		T *pdata = DARK_NEW T;
		if (pdata == 0) return 0;
		if( Insert(pTo, pdata) == 0 ) {
			delete pdata;
			return 0;
		}

		return pdata;
	}

	T *Insert(T *pTo, T *pThis) {
		pRyuLinkedListST pT, pT2;

		pT = m_pList;

		while(pT != 0) {
			if( (T *) pT->pData == pTo ) break;
			pT = pT->pNext;
		}

		if( pT == 0 ) return 0;

		pT2 = DARK_NEW RyuLinkedListST;
		if (pT2 == 0) return 0;
		pT2->pNext = pT->pNext;
		pT->pNext = pT2;
		pT2->pData = pThis;

		return pThis;
	}

	T *Insert(int iTo) {
		T *pdata = DARK_NEW T;
		if (pdata == 0) return 0;
		if (Insert(iTo, pdata) == 0) {
			delete pdata;
			return 0;
		}
		return pdata;
	}

	T *Insert(int iTo, T *pThis) {
		int iCnt;
		pRyuLinkedListST pT, pT2;

		pT = 0;
		iCnt = 0;
		while (iCnt < iTo) {
			if (pT == 0) pT = m_pList;
			else {
				pT = pT->pNext;
				if (pT == 0) break;
			}			
			iCnt++;
		}

		if (iCnt != iTo) return 0;

		pT2 = DARK_NEW RyuLinkedListST;
		if (pT == 0) {
			pT2->pNext = m_pList;
			pT2->pData = pThis;
			m_pList = pT2;
		}
		else {
			pT2->pNext = pT->pNext;
			pT->pNext = pT2;
			pT2->pData = pThis;
		}

		return pThis;
	}

	void Delete(T *pThis) {
		pRyuLinkedListST pT, pPre;

		pT = m_pList;
		pPre = pT;
		while(pT != 0) {
			if( (T *) pT->pData == pThis ) break;
			pPre = pT;
			pT = pT->pNext;
		}

		if( pT == 0 ) return;

		if( pT == m_pReadCursor ) m_pReadCursor = m_pReadCursor->pNext;
		if( pT == m_pList ) m_pList = m_pList->pNext;
		else pPre->pNext = pT->pNext;

		delete pThis;
		delete pT;
	}

	void DeleteAll() {
		pRyuLinkedListST pT;
		T *pdata;

		while(m_pList != 0)
		{
			pT = m_pList;
			m_pList = m_pList->pNext;

			pdata = (T *) pT->pData;
			if( pdata != 0 ) delete pdata;
			delete pT;
		}

		m_pReadCursor = 0;
	}

	void Rewind() {
		m_pReadCursor = m_pList;
	}

	T *GetNext(void) {
		T *pRes;
		if( m_pReadCursor == 0 ) return 0;

		pRes = (T *) m_pReadCursor->pData;
		m_pReadCursor = m_pReadCursor->pNext;

		return pRes;
	}

	T *GetNext(bool bFromFirst) {
		if( bFromFirst == true ) m_pReadCursor = m_pList;
		return GetNext();
	}

	bool IsExist(T *pThis) {
		bool bRes;
		pRyuLinkedListST pT = m_pList;
		bRes = false;

		while(pT != 0) {
			if( (T *) pT->pData == pThis ) {
				bRes = true;
				break;
			}
			pT = pT->pNext;
		}

		return bRes;
	}

	int GetCount() {
		int iRes;
		pRyuLinkedListST pT = m_pList;		

		iRes = 0;
		while(pT != 0) {
			iRes++;
			pT = pT->pNext;
		}
		return iRes;
	}

	void *GetCursor() { return (void *) m_pList; }

	T *GetNext(void **pCursor) {
		pRyuLinkedListST pT = (pRyuLinkedListST) (*pCursor);
		if( pT == 0 ) return 0;
		*pCursor = pT->pNext;

		return (T *) pT->pData;
	}

	T *GetAt(int iIndex) {
		int i = 0;
		T *pRes = 0;
		pRyuLinkedListST pT = m_pList;		
		while(pT != 0) {
			pRes = (T *) pT->pData;
			if( i == iIndex ) break;
			i++;
			pT = pT->pNext;
		}
		return pRes;
	}

	void Remove(T *pThis) {
		pRyuLinkedListST pT, pPre;

		pT = m_pList;
		pPre = pT;
		while (pT != 0) {
			if ((T *)pT->pData == pThis) break;
			pPre = pT;
			pT = pT->pNext;
		}

		if (pT == 0) return;

		if (pT == m_pReadCursor) m_pReadCursor = m_pReadCursor->pNext;
		if (pT == m_pList) m_pList = m_pList->pNext;
		else pPre->pNext = pT->pNext;

		delete pT;
	}

	void RemoveAll() {
		pRyuLinkedListST pT;

		while (m_pList != 0)
		{
			pT = m_pList;
			m_pList = m_pList->pNext;

			delete pT;
		}

		m_pReadCursor = 0;
	}

	bool MoveTo(T *pThis, int iPosition) {
		pRyuLinkedListST pT, pPre;

		pT = m_pList;
		pPre = pT;
		while (pT != 0) {
			if ((T *)pT->pData == pThis) break;
			pPre = pT;
			pT = pT->pNext;
		}

		if (pT == 0) return false;

		if (pT == m_pReadCursor) m_pReadCursor = m_pReadCursor->pNext;
		if (pT == m_pList) m_pList = m_pList->pNext;
		else pPre->pNext = pT->pNext;

		pT->pNext = 0;

		if (iPosition == 0 || m_pList == 0 ) {
			pT->pNext = m_pList;
			m_pList = pT;
		}
		else if (iPosition < 0) {
			pPre = m_pList;
			while (pPre->pNext != 0) pPre = pPre->pNext;
			pPre->pNext = pT;
		}
		else {
			int iPos = 1;
			pPre = m_pList;
			while (pPre != 0) {
				if (iPos == iPosition) break;
				pPre = pPre->pNext;
				iPos++;
			}
			if (pPre == 0) {
				delete pT; //데이터가 그냥 사라짐..
				return false;
			}
			pT->pNext = pPre->pNext;
			pPre->pNext = pT;
		}
		return true;
	}

private:
	pRyuLinkedListST m_pList;
	pRyuLinkedListST m_pReadCursor;

};

