#include "net_detector.h"

#define PR_ERR       printf
#define PR_DBG       printf

#ifndef SOCKET_ERROR
#define SOCKET_ERROR	(-1)
#endif

#define DARKNET_BROADCAST_TYPESTR		"HitekDPA_MTX"

netTXInfo::netTXInfo() {
	m_bVRRP = false;
	m_nTimestamp = 0;
}

netTXInfo::~netTXInfo() {
}

bool netTXInfo::check(Json::Value &jsInfo, uint32_t nTick) {
	if (m_dsUUID.m_uiSize == 0 || m_dsUUID.m_szString == NULL) return false;
	if (strcmp(m_dsUUID.m_szString, jsInfo["uuid"].asCString()) == 0) {
		m_nTimestamp = nTick;
		return true;
	}
	return false;
}

void netTXInfo::Reset(Json::Value &jsInfo, uint32_t nTick) {
	m_dsUUID.SetString(jsInfo["uuid"].asCString());
	m_dsID.SetString(jsInfo["id"].asCString());
	m_bVRRP = jsInfo["vrrp"].asBool();
	if (m_bVRRP) m_dsVRRP_ID.SetString(jsInfo["vrrp_id"].asCString());
	m_dsIP.SetString(jsInfo["ip"].asCString());
	m_nTimestamp = nTick;
}

bool netTXInfo::isTimedOut(uint32_t nTick) {
	if (m_nTimestamp == 0) return false;
	if (nTick > m_nTimestamp && nTick - m_nTimestamp > DARKNET_ALIVE_TIMEOUT) return true;
	return false;
}

bool netTXInfo::asJSON(Json::Value &OUT_jsV) {
	if (m_dsUUID.m_uiSize == 0 || m_dsUUID.m_szString == NULL) return false;
	OUT_jsV["uuid"] = m_dsUUID.m_szString;
	OUT_jsV["id"] = m_dsID.m_szString;
	OUT_jsV["ip"] = m_dsIP.m_szString;
	OUT_jsV["vrrp"] = m_bVRRP;
	if (m_bVRRP) OUT_jsV["vrrp_id"] = m_dsVRRP_ID.m_szString;
	return true;
}

//////////////////////////////////////////////////////////////////////////

netDetector::netDetector(pFunc_NetStateCallback pCBFN, void *pCBData) {
	m_pCBFN = pCBFN;
	m_pCBData = pCBData;

    m_socket = 0;
    m_iTerminate = 0;
	pthread_attr_init(&m_THRATT_detach);
	pthread_attr_setdetachstate(&m_THRATT_detach, PTHREAD_CREATE_DETACHED);
	DARKCRITICAL_INIT(m_lock);
}

netDetector::~netDetector() {
    if (m_socket > 0) DARK_NET_CLOSE(m_socket);
	DARKCRITICAL_DELETE(m_lock);
}

int netDetector::init(unsigned int nPort) {
	DARKSOCKET sockfd;
	struct sockaddr_in addr;
	struct timeval timeout;

	if (nPort == 0) return -1;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR) {
		PR_ERR("DGRAM Socket creation failed\n");
		return -2;
	}

	memset(&timeout, 0, sizeof(struct timeval));
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(struct timeval)) == SOCKET_ERROR) {
		PR_ERR("SO_RCVTIMEO failed\n");
		DARK_NET_CLOSE(sockfd);
		return -3;
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(nPort);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sockfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) == SOCKET_ERROR) {
		PR_ERR("bind failed\n");
		DARK_NET_CLOSE(sockfd);
		return -4;
	}

	m_socket = sockfd;

	m_iTerminate = 1;
	pthread_t pThr;
	if (pthread_create(&pThr, &m_THRATT_detach, THR_Receiver, this) != 0) {
		PR_ERR("Receiver thread creation failed!!!\n");                
		return -5;
	}

	return 0;    
}

void* netDetector::THR_Receiver(void* pParam)
{
	netDetector* pP = (netDetector*)pParam;
	pP->thr_Receiver();
	return 0;
}

void netDetector::thr_Receiver() 
{
	struct sockaddr_in addr;
	socklen_t slen;
	char *szBuf = new char[DARK_MAXPATH];
	char szSender[255];
	int iRecv;
	uint32_t nLastCheck = DARKPIF_GetTickCount();
	uint32_t nTick = 0;
	netTXInfo *pTX;
	bool bChanged = false;

	PR_DBG("Start receiver!!!!!\n");

	Json::Value jsR;
	Json::Reader reader;

	while (!(m_iTerminate & 0x02)) {
		memset(&addr, 0, sizeof(struct sockaddr_in));
		memset(szBuf, 0, DARK_MAXPATH);
		slen = sizeof(struct sockaddr_in);
		iRecv = recvfrom(m_socket, szBuf, DARK_MAXPATH, 0, (struct sockaddr*)&addr, &slen);
		if (iRecv <= 0) {
		}		
		else {
			szSender[0] = 0;
			inet_ntop(AF_INET, &(addr.sin_addr.s_addr), szSender, 255);
			
			if (!reader.parse(szBuf, jsR)) PR_ERR("ERROR: Json parsing failed from %s\n%s\n", szSender, szBuf);
			else onPacketReceived(szSender, jsR);
		}

		nTick = DARKPIF_GetTickCount();
		if (nTick > nLastCheck && nTick - nLastCheck > 1000) {						
			bChanged = false;
			DARKCRITICAL_LOCK(m_lock);
			m_MTXList.Rewind();
			while((pTX = m_MTXList.GetNext()) != NULL) {
				if (pTX->isTimedOut(nTick)) {
					bChanged = true;
					m_MTXList.Delete(pTX);
				}
			}
			DARKCRITICAL_UNLOCK(m_lock);
			nLastCheck = nTick;
			if (bChanged) {
				m_pCBFN(DARKNET_EVT_MTX_DISAPPERED, NULL, m_pCBData);
			}
		}
	}
	m_iTerminate |= 0x04;

	PR_ERR("Receiver terminated!!!!!\n");
	delete[] szBuf;
}

void netDetector::onPacketReceived(const char *szIP_From, Json::Value &jsRecv) {
	if ( !jsRecv.isMember("type") || !jsRecv.isMember("uuid")) {
		PR_ERR("Invalid json...\n");
		return;
	}
	if (strcmp(jsRecv["type"].asCString(), DARKNET_BROADCAST_TYPESTR) == 0) {
		bool bExist;
		uint32_t nTick;
		netTXInfo *pT;

		DARKCRITICAL_LOCK(m_lock);		
		m_MTXList.Rewind();
		bExist = false;
		nTick = DARKPIF_GetTickCount();
		
		while((pT = m_MTXList.GetNext()) != NULL) {
			if (pT->check(jsRecv, nTick)) {
				bExist = true;
				break;
			}
		}
		
		if (!bExist) {
			pT = m_MTXList.Add();
			pT->Reset(jsRecv, nTick);			
		}
		DARKCRITICAL_UNLOCK(m_lock);

		if (!bExist) {
			m_pCBFN(DARKNET_EVT_NEW_MTX_DETECTED, NULL, m_pCBData);
		}
	}
}

Json::Value netDetector::getMTXList(int &OUT_iCnt) {
	netTXInfo *pT;
	Json::Value jsV, jsV2;
	
	DARKCRITICAL_LOCK(m_lock);
	OUT_iCnt = 0;
	m_MTXList.Rewind();
	while((pT = m_MTXList.GetNext()) != NULL) {
		jsV2.clear();
		if(pT->asJSON(jsV2)) {
			OUT_iCnt++;
			jsV.append(jsV2);
		}
	}
	DARKCRITICAL_UNLOCK(m_lock);

	return jsV;
}


