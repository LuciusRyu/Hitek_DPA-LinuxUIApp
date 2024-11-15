#include "config_remote.h"
#include "Information_Parser.h"

#define PR_ERR(fmt, args...)	printf("[CONF_REMOTE_ERROR]: " fmt, ##args)
#define PR_DBG(fmt, args...)	printf("[CONF_REMOTE_DEBUG]: " fmt, ##args)

configRemote::configRemote() {
    m_cszBasePath = NULL;
    m_szTemp = new char[DARK_MAXPATH];
    CONF_szDanteID = new char[DARK_MIDSTR_S];
    CONF_UART_szDev = new char[DARK_MIDSTR_S];
    m_szTemp[0] = 0;
    CONF_szDanteID[0] = 0;
    CONF_szLocalMediaPath = new char[DARK_MAXPATH];
    snprintf(CONF_szLocalMediaPath, DARK_MAXPATH, "/HitekDPA/local_web/local_media");
    snprintf(CONF_UART_szDev, DARK_MIDSTR_S, "/dev/ttyS4");
    CONF_UART_iBaud = 115200;

    m_pDDU = NULL;
    m_pUART = NULL;
}

configRemote::~configRemote() {
    if (m_szTemp != NULL) delete[] m_szTemp;
    if (CONF_szDanteID != NULL) delete[] CONF_szDanteID;
    if (CONF_szLocalMediaPath != NULL) delete[] CONF_szLocalMediaPath;
    if (CONF_UART_szDev != NULL) delete[] CONF_UART_szDev;

    if (m_pDDU != NULL) delete m_pDDU;
    if (m_pUART != NULL) delete m_pUART;
}

int configRemote::init(const char *szBasePath) {    
    m_cszBasePath = szBasePath;    

    Information_Parser IP;
    pTAGInformation pRoot, pT1, pT2;
    int iRes = 0;

    pRoot = OpenConfig(IP);
    if (pRoot != NULL) {
        if ((pT1 = IP.GetTag("uart_port", pRoot)) != NULL) snprintf(CONF_UART_szDev, DARK_MIDSTR_S, "%s", pT1->pBuffer);
        if ((pT1 = IP.GetTag("uart_baud", pRoot)) != NULL) CONF_UART_iBaud = atoi((const char *)pT1->pBuffer);
        if((pT1 = IP.GetTag("local_media_path", pRoot)) != NULL) snprintf(CONF_szLocalMediaPath, DARK_MAXPATH, "%s", pT1->pBuffer);
        iRes = 1;
    }    
    
    if (m_pUART == NULL) {
        m_pUART = new UART_Serial();
        if (m_pUART->Open(CONF_UART_szDev, CONF_UART_iBaud, 0, 8, 0) != 0) {
            PR_ERR("ERROR: UART - Opening failed..[%s:%d]\n", CONF_UART_szDev, CONF_UART_iBaud);
            return 0;
        }
        PR_DBG("UART %s Open as %d success", CONF_UART_szDev, CONF_UART_iBaud);
    }
    if (m_pDDU == NULL) {
        m_pDDU = new Dark_Dante_UHIP();
        if (m_pDDU->SetTransport(m_pUART) != 0) {
            PR_ERR("ERROR: UART - Transport failed\n");
            delete m_pUART;
            delete m_pDDU;
            m_pUART = NULL;
            m_pDDU = NULL;
            return 0;
        }
        DARK_SLEEP(100);
        m_pDDU->GetDeviceInfo(false, NULL); //UART 디바이스 정보 요청
    }
    if (iRes > 0) return 100;
    return 0;
}

bool configRemote::save() {
    FILE *fp = NULL;

    snprintf(m_szTemp, DARK_MAXPATH, "%s/remote_conf.ifp", m_cszBasePath);
    if (DARKPIF_fopen(&fp, m_szTemp, "w+") != true) {
        PR_ERR("ERROR: Save - Opening config file..[%s]\n", m_szTemp);
        return false;
    }

    fprintf(fp, "<HITEK_REMOTE_UI>\n");
    fprintf(fp, "\t<uart_port>%s</uart_port>\n", CONF_UART_szDev);
    fprintf(fp, "\t<uart_baud>%d</uart_baud>\n", CONF_UART_iBaud);
    fprintf(fp, "\t<local_media_path>%s</local_media_path>\n", CONF_szLocalMediaPath);
    fprintf(fp, "</HITEK_REMOTE_UI>\n");

    fclose(fp);
    return true;
}

bool configRemote::getSetupPayload(Json::Value &OUT_payload) {        
    Dark_Dante_DevInfo *pDDD;
    if (m_pDDU == NULL) {
        OUT_payload["status"] = "ERROR";
        OUT_payload["error"] = "UltimoX UART is not opened";
        return true;
    }
    if (m_pDDU->GetDeviceInfo(false, &pDDD) != 0) {
        OUT_payload["status"] = "ING";
        OUT_payload["error"] = "UART Communicate is in progress";
        return true;
    }
    if (pDDD->m_szDevName != NULL) strcpy(CONF_szDanteID, pDDD->m_szDevName);
    if (strlen(CONF_szDanteID) > 0) OUT_payload["dante_dev_id"] = CONF_szDanteID;
    else OUT_payload["dante_dev_id"] = "NONE";

    Json::Value jsDante;

	jsDante["device_id"] = pDDD->m_szDevId;    
	if (pDDD->m_szModel_id != NULL) jsDante["model_id"] = pDDD->m_szModel_id;
	else jsDante["model_id"] = "NONE";
    jsDante["software_version"] = pDDD->m_sw_ver;
	jsDante["software_build"] = pDDD->m_sw_build;
	jsDante["firmware_version"] = pDDD->m_fw_ver;
	jsDante["firmware_build"] = pDDD->m_fw_build;
	jsDante["bootloader_version"] = pDDD->m_bootloader_ver;
	jsDante["bootloader_build"] = pDDD->m_bootloader_build;
	jsDante["api_version"] = pDDD->m_api_ver;
	jsDante["capa_flags"] = pDDD->m_cap_flags;
	jsDante["status_flags"] = pDDD->m_status_flags;	

    OUT_payload["status"] = "OK";
    OUT_payload["dante_dev_info"] = jsDante;
    return true;
}

bool configRemote::setSetupPayload(Json::Value payload) {    
    //DARK_STRCPY(CONF_szDanteID, DARK_MIDSTR_S, payload["dante_dev_id"].asCString());
    return save();
}

pTAGInformation configRemote::OpenConfig(Information_Parser &IFP) {
    pTAGInformation pRoot;
    snprintf(m_szTemp, DARK_MAXPATH, "%s/remote_conf.ifp", m_cszBasePath);
    if (!IFP.OpenInfoFile(m_szTemp)) {
        PR_ERR("WARN: Init - Opening config file..[%s]\n", m_szTemp);
        return NULL;
    }

    pRoot = IFP.GetTag("HITEK_REMOTE_UI");
    if (pRoot == NULL) {
        PR_ERR("ERROR: Not valid config file...\n");
        return NULL;
    }

    return pRoot;
}
