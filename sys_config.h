#pragma once
#include "config_remote.h"
#include "config_maintx.h"
#include "config_int_cnc.h"


class sysConfig {
public:
    sysConfig();
    ~sysConfig();

    bool init(const char *szBasePath = NULL);
    bool saveBase();
    DARK_SYS_TYPE sysType() { return m_eType; }    
    const char *getSetupURI() { return CONF_szSetupURI; }    
    const char *getIndexURI() { return CONF_szIndexURI; }    
    int getNetPort() { return CONF_net_iBroadcast; }
    bool IsConfigured();
    bool getBaseSetupPayload(Json::Value &OUT_payload);
    bool setBaseSetupPayload(Json::Value payload);
    bool getIPAddress(char *OUT_szIP, int iBufSize);

    bool getSetupPayload(Json::Value &OUT_payload);
    bool setSetupPayload(Json::Value payload);
    bool getLoginPayload(Json::Value &OUT_payload);
    bool setLoginPayload(Json::Value payload);

    const char *getLocalMediaPath();
    pTAGInformation OpenConfig(Information_Parser &IFP);

protected:
    DARK_SYS_TYPE m_eType;
    char *m_szBasePath;
    char *m_szTemp;

    char *CONF_szSysID;
    char *CONF_szSetupURI;
    char *CONF_szIndexURI;
    int CONF_iConfigured;
    int CONF_net_iBroadcast;
    subConfigNet CONF_net;

    configRemote *m_conf_remote;
    configMaintx *m_conf_maintx;
    configIntCNC *m_conf_cnc;

    //bool _sys_setup_netif(bool bDHCP, const char *szIf, const char *szIP, const char *szMask, const char *szGateway, const char *szDNS1, const char *szDNS2);
};