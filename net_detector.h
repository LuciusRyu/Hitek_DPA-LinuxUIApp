#pragma once
#include "common_inc.h"
#include "RyuLinkedList.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

typedef int DARKSOCKET;
typedef void(*pFunc_NetStateCallback)(int32_t iState, void* pObject, void* pCBData);

#define DARK_NET_CLOSE(x) close(x)
#define DARKNET_ALIVE_TIMEOUT   10000

#define DARKNET_EVT_NEW_MTX_DETECTED    1
#define DARKNET_EVT_MTX_DISAPPERED       2

class netTXInfo {
public:
    netTXInfo();
    ~netTXInfo();

    bool check(Json::Value &jsInfo, uint32_t nTick);
    void Reset(Json::Value &jsInfo, uint32_t nTick);
    bool isTimedOut(uint32_t nTick);
    bool asJSON(Json::Value &OUT_jsV);

protected:
    DarkString m_dsID;
    DarkString m_dsUUID;
    DarkString m_dsIP;
    bool m_bVRRP;
    DarkString m_dsVRRP_ID;

    uint32_t m_nTimestamp;
};

class netDetector {
public:
    netDetector(pFunc_NetStateCallback pCBFN, void *pCBData);
    ~netDetector();

    int init(unsigned int nPort);
    Json::Value getMTXList(int &OUT_iCnt);

protected:
    pFunc_NetStateCallback m_pCBFN;
    void *m_pCBData;

    DARKSOCKET m_socket;
	pthread_attr_t m_THRATT_detach;
    int m_iTerminate;    
    RyuLinkedList<netTXInfo> m_MTXList;
    DARKCRITICAL_TYPE m_lock;

	static void* THR_Receiver(void* pParam);
    void thr_Receiver();
    void onPacketReceived(const char *szIP_From, Json::Value &jsRecv);
};
