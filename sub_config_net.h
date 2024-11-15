#pragma once
#include "common_inc.h"

class subConfigNet {
public:
    subConfigNet();
    ~subConfigNet();

    int init(void *pTag);
    bool save(FILE *fp, int iTap = 2);
    bool getPayload(Json::Value &OUT_payload);
    bool setPayload(Json::Value payload);

    char *m_szInterface;

protected:
    bool m_bIsStaticIP;    
    char *m_szIP;
    char *m_szNetmask;
    char *m_szGateway;
    char *m_szDNS1;
    char *m_szDNS2;

    bool _sys_setup_netif(bool bDHCP, const char *szIf, const char *szIP, const char *szMask, const char *szGateway, const char *szDNS1, const char *szDNS2);
};
