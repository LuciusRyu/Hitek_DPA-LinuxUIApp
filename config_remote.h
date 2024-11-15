#pragma once
#include "common_inc.h"
#include "Dark_Dante_UHIP.h"
#include "UART_Serial.h"

class configRemote {
public:
    configRemote();
    ~configRemote();

    int init(const char *szBasePath);
    bool save();
    bool getSetupPayload(Json::Value &OUT_payload);
    bool setSetupPayload(Json::Value payload);
    const char *getLocalMediaPath() { return CONF_szLocalMediaPath; }
    pTAGInformation OpenConfig(Information_Parser &IFP);

protected:
    const char *m_cszBasePath;
    char *m_szTemp;

    char *CONF_szDanteID;
    char *CONF_szLocalMediaPath;
    char *CONF_UART_szDev;
    int CONF_UART_iBaud;

    UART_Serial *m_pUART;
    Dark_Dante_UHIP *m_pDDU;
};
