#pragma once
#include "common_inc.h"
#include "sub_config_cluster.h"
#include "sub_config_net.h"

class configMaintx {
public:
    configMaintx();
    ~configMaintx();

    int init(const char *szBasePath);
    bool save(bool bSetupVRRP = true);
    bool getSetupPayload(Json::Value &OUT_payload);
    bool setSetupPayload(Json::Value payload);
    bool getLoginPayload(Json::Value &OUT_payload);
    bool setLoginPayload(Json::Value payload);

    pTAGInformation OpenConfig(Information_Parser &IFP);

protected:
    const char *m_cszBasePath;
    char *m_szTemp;

    char *CONF_szDanteID;
    bool CONF_vrrp_bUse;
    int CONF_vrrp_iID;
    bool CONF_vrrp_bIsMaster;
    int CONF_vrrp_iPriority;
    char *CONF_vrrp_szInf;
    char *CONF_vrrp_szIP;

    bool CONF_localogin_bAuto;
    char *CONF_localogin_szID;
    char *CONF_localogin_szPW;
    subConfigCluster CONF_cluster;

    char *CONF_uart_szDevAddr;
    int CONF_uart_iBaudPort;
    subConfigNet CONF_uart_net;

    bool _sys_setup_vrrp();
};
