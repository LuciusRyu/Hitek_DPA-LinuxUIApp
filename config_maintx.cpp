#include "config_maintx.h"
#include "Information_Parser.h"

#define PR_ERR       printf
#define PR_WARN      printf

configMaintx::configMaintx() {
    m_cszBasePath = NULL;
    m_szTemp = new char[DARK_MAXPATH];
    CONF_szDanteID = new char[DARK_MIDSTR_S];
    m_szTemp[0] = 0;
    CONF_szDanteID[0] = 0;

    CONF_vrrp_bUse = false;
    CONF_vrrp_iID = 0;
    CONF_vrrp_bIsMaster = false;
    CONF_vrrp_iPriority = 0;
    CONF_vrrp_szInf = NULL;
    CONF_vrrp_szIP = NULL;

    CONF_localogin_bAuto = false;
    CONF_localogin_szID = new char[DARK_MIDSTR_S];
    CONF_localogin_szPW = new char[DARK_MAXPATH];
    snprintf(CONF_localogin_szID, DARK_MIDSTR_S, "NONE");
    snprintf(CONF_localogin_szPW, DARK_MAXPATH, "NONE");

    CONF_uart_szDevAddr = new char[DARK_MIDSTR_S];
    snprintf(CONF_uart_szDevAddr, DARK_MIDSTR_S, "/dev/ttyS3");
    CONF_uart_iBaudPort = 9600;
}

configMaintx::~configMaintx() {
    if (m_szTemp != NULL) delete[] m_szTemp;
    if (CONF_szDanteID != NULL) delete[] CONF_szDanteID;
    if (CONF_vrrp_szInf != NULL) delete[] CONF_vrrp_szInf;
    if (CONF_vrrp_szIP != NULL) delete[] CONF_vrrp_szIP;
    if (CONF_localogin_szID != NULL) delete[] CONF_localogin_szID;
    if (CONF_localogin_szPW != NULL) delete[] CONF_localogin_szPW;
    if (CONF_uart_szDevAddr != NULL) delete[] CONF_uart_szDevAddr;
}

int configMaintx::init(const char *szBasePath) {    
    m_cszBasePath = szBasePath;    

    Information_Parser IP;
    pTAGInformation pRoot, pT1, pT2;

    pRoot = OpenConfig(IP);
    if (pRoot == NULL) return 0;

    pT1 = IP.GetTag("dante_dev_id", pRoot);
    if (pT1 == NULL) {
        PR_WARN("WARN: No dante id\n");
        return 1;
    }
    snprintf(CONF_szDanteID, DARK_MIDSTR_S, "%s", pT1->pBuffer);

    pT1 = IP.GetTag("use_vrrp", pRoot);
    if (pT1 != NULL) {
        if (strcmp((char *)pT1->pBuffer, "true") == 0) CONF_vrrp_bUse = true;
        else CONF_vrrp_bUse = false;
    }

    pT1 = IP.GetTag("VRRP_INFO", pRoot);
    if (CONF_vrrp_bUse) {
        if (pT1 == NULL) {
            PR_ERR("ERROR: Not valid config file(vrrp)...\n");
            return 0;
        }
    }
    if (pT1 != NULL) {
        if (CONF_vrrp_szInf == NULL) CONF_vrrp_szInf = new char[DARK_MIDSTR_S];
        if (CONF_vrrp_szIP == NULL) CONF_vrrp_szIP = new char[DARK_MIDSTR_S];
        CONF_vrrp_szInf[0] = 0;
        CONF_vrrp_szIP[0] = 0;

        if ((pT2 = IP.GetTag("id", pT1)) != NULL) CONF_vrrp_iID = atoi((char *)pT2->pBuffer);
        if ((pT2 = IP.GetTag("master", pT1)) != NULL) {
            if (strcmp((char *)pT2->pBuffer, "yes") == 0) CONF_vrrp_bIsMaster = true;
            else CONF_vrrp_bIsMaster = false;
        }
        if ((pT2 = IP.GetTag("priority", pT1)) != NULL) CONF_vrrp_iPriority = atoi((char *)pT2->pBuffer);
        if ((pT2 = IP.GetTag("interface", pT1)) != NULL) DARK_STRCPY(CONF_vrrp_szInf, DARK_MIDSTR_S, (char *)pT2->pBuffer);
        if ((pT2 = IP.GetTag("ip", pT1)) != NULL) DARK_STRCPY(CONF_vrrp_szIP, DARK_MIDSTR_S, (char *)pT2->pBuffer);

        CONF_cluster.init((void *)pT1);
    }
    pT1 = IP.GetTag("uart", pRoot);
    if (pT1 != NULL) {
        if ((pT2 = IP.GetTag("port", pT1)) != NULL) DARK_STRCPY(CONF_uart_szDevAddr, DARK_MIDSTR_S, (char *)pT2->pBuffer);
        if ((pT2 = IP.GetTag("baudrate", pT1)) != NULL) CONF_uart_iBaudPort = atoi((char *)pT2->pBuffer);
        if ((pT2 = IP.GetTag("net", pT1)) != NULL) CONF_uart_net.init(pT2);
    }
    else snprintf(CONF_uart_szDevAddr, DARK_MIDSTR_S, "NONE");

    pT1 = IP.GetTag("LOCAL_LOGIN", pRoot);
    if (pT1 == NULL) {
        PR_WARN("WARN: No Local login info\n");
        return 2;
    }

    if ((pT2 = IP.GetTag("auto", pT1)) != NULL) {
        if (strcmp((char *)pT2->pBuffer, "yes") == 0) CONF_localogin_bAuto = true;
        else CONF_localogin_bAuto = false;
    }
    if ((pT2 = IP.GetTag("ID", pT1)) != NULL) DARK_STRCPY(CONF_localogin_szID, DARK_MIDSTR_S, (char *)pT2->pBuffer);
    if ((pT2 = IP.GetTag("PW", pT1)) != NULL) DARK_STRCPY(CONF_localogin_szPW, DARK_MAXPATH, (char *)pT2->pBuffer);
    
    return 100;
}

bool configMaintx::save(bool bSetupVRRP) {
    FILE *fp = NULL;

    snprintf(m_szTemp, DARK_MAXPATH, "%s/main_conf.ifp", m_cszBasePath);
    if (DARKPIF_fopen(&fp, m_szTemp, "w+") != true) {
        PR_ERR("ERROR: Save - Opening config file..[%s]\n", m_szTemp);
        return false;
    }

    fprintf(fp, "<HITEK_MAIN_UI>\n");
    fprintf(fp, "\t<dante_dev_id>%s</dante_dev_id>\n", CONF_szDanteID);
    fprintf(fp, "\t<use_vrrp>%s</use_vrrp>\n", CONF_vrrp_bUse ? "true" : "false");
    if (CONF_vrrp_bUse || CONF_vrrp_szInf != NULL || CONF_vrrp_szIP != NULL) {
        fprintf(fp, "\t<VRRP_INFO>\n");
        fprintf(fp, "\t\t<id>%d</id>\n", CONF_vrrp_iID);
        fprintf(fp, "\t\t<master>%s</master>\n", CONF_vrrp_bIsMaster ? "yes" : "no");
        fprintf(fp, "\t\t<priority>%d</priority>\n", CONF_vrrp_iPriority);
        fprintf(fp, "\t\t<interface>%s</interface>\n", CONF_vrrp_szInf);
        fprintf(fp, "\t\t<ip>%s</ip>\n", CONF_vrrp_szIP);
        CONF_cluster.save(fp);
        fprintf(fp, "\t</VRRP_INFO>\n");
    }
    if (strcmp(CONF_uart_szDevAddr, "NONE") != 0) {
        fprintf(fp, "\t<uart>\n");        
        fprintf(fp, "\t\t<id>1</td>\n");        
        fprintf(fp, "\t\t<type>fire</type>\n");        
        fprintf(fp, "\t\t<port>%s</port>\n", CONF_uart_szDevAddr);        
        fprintf(fp, "\t\t<baudrate>%d</baudrate>\n", CONF_uart_iBaudPort);        
        if (strncmp(CONF_uart_szDevAddr, "ETH:", 4) == 0) {
            fprintf(fp, "\t\t<net>\n");        
            CONF_uart_net.save(fp, 3);
            fprintf(fp, "\t\t</net>\n");        
        }
        fprintf(fp, "\t</uart>\n");        
    }
    fprintf(fp, "\t<LOCAL_LOGIN>\n");
    fprintf(fp, "\t\t<auto>%s</auto>\n", CONF_localogin_bAuto ? "yes" : "no");
    fprintf(fp, "\t\t<ID>%s</ID>\n", strlen(CONF_localogin_szID) > 0 ? CONF_localogin_szID : "NONE");
    fprintf(fp, "\t\t<PW>%s</PW>\n", strlen(CONF_localogin_szPW) > 0 ? CONF_localogin_szPW : "NONE");
    fprintf(fp, "\t</LOCAL_LOGIN>\n");
    fprintf(fp, "</HITEK_MAIN_UI>\n");

    fclose(fp);
    if (bSetupVRRP) return _sys_setup_vrrp();
    return true;
}

bool configMaintx::getSetupPayload(Json::Value &OUT_payload) {        
    if (strlen(CONF_szDanteID) > 0) OUT_payload["dante_dev_id"] = CONF_szDanteID;
    else OUT_payload["dante_dev_id"] = "NONE";
    if (CONF_vrrp_bUse) {
        Json::Value vrrp;        
        vrrp["id"] = CONF_vrrp_iID;
        vrrp["is_master"] = CONF_vrrp_bIsMaster;
        vrrp["priority"] = CONF_vrrp_iPriority;
        vrrp["interface"] = CONF_vrrp_szInf;
        vrrp["ip"] = CONF_vrrp_szIP;
        CONF_cluster.getPayload(vrrp);

        OUT_payload["use_vrrp"] = true;
        OUT_payload["vrrp"] = vrrp;
    }
    else OUT_payload["use_vrrp"] = false;
    Json::Value uart;
    if (strcmp(CONF_uart_szDevAddr, "NONE") == 0) uart["type"] = "NONE";
    else {
        if (strncmp(CONF_uart_szDevAddr, "ETH:", 4) == 0) {
            uart["type"] = "TCPIP";
            Json::Value net;
            CONF_uart_net.getPayload(net);
            uart["net"] = net;
            uart["target_ip"] = CONF_uart_szDevAddr + 4;
            uart["target_port"] = CONF_uart_iBaudPort;
        }
        else {
            uart["type"] = "RS232";
            uart["dev_name"] = CONF_uart_szDevAddr;
            uart["baudrate"] = CONF_uart_iBaudPort;
        }
    }
    OUT_payload["uart"] = uart;
    return true;
}

bool configMaintx::setSetupPayload(Json::Value payload) {    
    DARK_STRCPY(CONF_szDanteID, DARK_MIDSTR_S, payload["dante_dev_id"].asCString());
    CONF_vrrp_bUse = payload["use_vrrp"].asBool();
    if (CONF_vrrp_bUse) {
        Json::Value vrrp = payload["vrrp"];
        CONF_vrrp_iID = vrrp["id"].asInt();
        CONF_vrrp_bIsMaster = vrrp["is_master"].asBool();
        CONF_vrrp_iPriority = vrrp["priority"].asInt();

        if (CONF_vrrp_szInf == NULL) CONF_vrrp_szInf = new char[DARK_MIDSTR_S];
        if (CONF_vrrp_szIP == NULL) CONF_vrrp_szIP = new char[DARK_MIDSTR_S];
        CONF_vrrp_szInf[0] = 0;
        CONF_vrrp_szIP[0] = 0;

        DARK_STRCPY(CONF_vrrp_szInf, DARK_MIDSTR_S, vrrp["interface"].asCString());
        DARK_STRCPY(CONF_vrrp_szIP, DARK_MIDSTR_S, vrrp["ip"].asCString());
        CONF_cluster.setPayload(vrrp);
    }
    if (payload.isMember("uart")) {
        Json::Value uart = payload["uart"];
        if (strcmp(uart["type"].asCString(), "TCPIP") == 0) {
            if (uart.isMember("net")) CONF_uart_net.setPayload(uart["net"]);
            snprintf(CONF_uart_szDevAddr, DARK_MIDSTR_S, "ETH:%s", uart["target_ip"].asCString());
            CONF_uart_iBaudPort = uart["target_port"].asInt();
        }
        else if (strcmp(uart["type"].asCString(), "RS232") == 0) {
            DARK_STRCPY(CONF_uart_szDevAddr, DARK_MIDSTR_S, uart["dev_name"].asCString());
            CONF_uart_iBaudPort = uart["baudrate"].asInt();
        }
        else {
            snprintf(CONF_uart_szDevAddr, DARK_MIDSTR_S, "NONE");            
        }
    }
    else snprintf(CONF_uart_szDevAddr, DARK_MIDSTR_S, "NONE");
    return save();
}

bool configMaintx::_sys_setup_vrrp() {
    snprintf(m_szTemp, DARK_MAXPATH, "%s/vrrp_conf.sh", m_cszBasePath);
    return GCMN_setup_vrrp(m_szTemp, CONF_vrrp_bUse, CONF_vrrp_bIsMaster, CONF_vrrp_iID, CONF_vrrp_iPriority, CONF_vrrp_szInf, CONF_vrrp_szIP);
}

bool configMaintx::getLoginPayload(Json::Value &OUT_payload) {        
    OUT_payload["auto"] = CONF_localogin_bAuto;
    if (strlen(CONF_localogin_szID) > 0) OUT_payload["id"] = CONF_localogin_szID;
    else OUT_payload["id"] = "NONE";
    if (strlen(CONF_localogin_szPW) > 0) OUT_payload["pw"] = CONF_localogin_szPW;
    else OUT_payload["pw"] = "NONE";
    return true;
}

bool configMaintx::setLoginPayload(Json::Value payload) {    
    CONF_localogin_bAuto = payload["auto"].asBool();
    DARK_STRCPY(CONF_localogin_szID, DARK_MIDSTR_S, payload["id"].asCString());
    DARK_STRCPY(CONF_localogin_szPW, DARK_MIDSTR_S, payload["pw"].asCString());
    return save(false);
}

pTAGInformation configMaintx::OpenConfig(Information_Parser &IFP) {
    pTAGInformation pRoot;
    snprintf(m_szTemp, DARK_MAXPATH, "%s/main_conf.ifp", m_cszBasePath);
    if (!IFP.OpenInfoFile(m_szTemp)) {
        PR_ERR("WARN: Init - Opening config file..[%s]\n", m_szTemp);
        return NULL;
    }

    pRoot = IFP.GetTag("HITEK_MAIN_UI");
    if (pRoot == NULL) {
        PR_ERR("ERROR: Not valid config file...\n");
        return NULL;
    }

    return pRoot;
}
