#include "sys_config.h"
#include "Information_Parser.h"

#define PR_ERR       printf

sysConfig::sysConfig() {
    m_eType = DARK_SYS_TYPE::NONE;
    m_szBasePath = new char[DARK_MAXPATH];
    m_szTemp = new char[DARK_MAXPATH];
    snprintf(m_szBasePath, DARK_MAXPATH, "./config");


    CONF_net_iBroadcast = 40311;
    CONF_szSysID = new char[DARK_MIDSTR_S];
    CONF_szSysID[0] = 0;
    CONF_szSetupURI = new char[DARK_MAXPATH];
    CONF_szSetupURI[0] = 0;
    CONF_szIndexURI = new char[DARK_MAXPATH];
    CONF_szIndexURI[0] = 0;

    CONF_iConfigured = 0;

    m_conf_remote = NULL;
    m_conf_maintx = NULL;
    m_conf_cnc = NULL;
}

sysConfig::~sysConfig() {
    delete[] m_szBasePath;
    delete[] m_szTemp;
    if (CONF_szSysID != NULL) delete[] CONF_szSysID;
    if (CONF_szSetupURI != NULL) delete[] CONF_szSetupURI;
    if (CONF_szIndexURI != NULL) delete[] CONF_szIndexURI;

    if (m_conf_remote != NULL) delete m_conf_remote;
    if (m_conf_maintx != NULL) delete m_conf_maintx;
    if (m_conf_cnc != NULL) delete m_conf_cnc;
}

bool sysConfig::IsConfigured() {
    if (CONF_iConfigured >= 100) return true;
    return false;
}

bool sysConfig::init(const char *szBasePath) {
    if (szBasePath != NULL) {
        snprintf(m_szBasePath, DARK_MAXPATH, "%s", szBasePath);
    }

    Information_Parser IP;
    pTAGInformation pRoot, pT1, pT2, pT3;
    int i;

    snprintf(m_szTemp, DARK_MAXPATH, "%s/conf.ifp", m_szBasePath);
    if (!IP.OpenInfoFile(m_szTemp)) {
        PR_ERR("ERROR: Init - Opening config file..[%s]\n", m_szTemp);
        return false;
    }

    pRoot = IP.GetTag("HITEK_LINUX_UI");
    if (pRoot == NULL) {
        PR_ERR("ERROR: Not valid config file...\n");
        return false;
    }
    pT1 = IP.GetTag("type", pRoot);
    if (pT1 == NULL) {
        PR_ERR("ERROR: Not valid config file...(no type)\n");
        return false;
    }
    if (strcmp((char *)pT1->pBuffer, "MAIN_TX") == 0) m_eType = DARK_SYS_TYPE::MAIN_TX;
    else if (strcmp((char *)pT1->pBuffer, "REMOTE") == 0) m_eType = DARK_SYS_TYPE::REMOTE;
    else if (strcmp((char *)pT1->pBuffer, "INT_CNC") == 0) m_eType = DARK_SYS_TYPE::INT_CNC;
    else {
        PR_ERR("ERROR: Not valid config file...(Unknown type:%s)\n", pT1->pBuffer);
        return false;
    }

    pT1 = IP.GetTag("setup_uri", pRoot);
    if (pT1 == NULL) {
        PR_ERR("ERROR: Not valid config file...(no setup uri)\n");
        return false;
    }
    snprintf(CONF_szSetupURI, DARK_MAXPATH, "%s", pT1->pBuffer);
    pT1 = IP.GetTag("index_uri", pRoot);
    if (pT1 == NULL) {
        PR_ERR("ERROR: Not valid config file...(no index uri)\n");
        return false;
    }
    snprintf(CONF_szIndexURI, DARK_MAXPATH, "%s", pT1->pBuffer);

    pT1 = IP.GetTag("status", pRoot);
    if (pT1 == NULL) {
        PR_ERR("ERROR: Not valid config file...(no status)\n");
        return false;
    }
    CONF_iConfigured = atoi((char *)pT1->pBuffer);    

    if ((pT1 = IP.GetTag("sys_id", pRoot)) != NULL) snprintf(CONF_szSysID, DARK_MIDSTR_S, "%s", pT1->pBuffer);

    pT1 = IP.GetTag("network", pRoot);
    if (pT1 == NULL) {
        PR_ERR("ERROR: Not valid config file...(no network)\n");
        return false;
    }

    if ((pT2 = IP.GetTag("broadcast_port", pT1)) != NULL) CONF_net_iBroadcast = atoi((char *) pT2->pBuffer);
    if (CONF_net.init(pT1) < 0) {
        PR_ERR("ERROR: Not valid config file... (net %d)\n", CONF_net.init(pT1));
        return false;
    }

    if (m_eType == DARK_SYS_TYPE::REMOTE) {
        m_conf_remote = new configRemote();        
        i =  m_conf_remote->init(m_szBasePath);
        if (CONF_iConfigured > 0) CONF_iConfigured = i;
    }
    else if (m_eType == DARK_SYS_TYPE::MAIN_TX) {
        m_conf_maintx = new configMaintx();
        i = m_conf_maintx->init(m_szBasePath);
        if (CONF_iConfigured > 0) CONF_iConfigured = i;
    }
    else if (m_eType == DARK_SYS_TYPE::INT_CNC) {
        m_conf_cnc = new configIntCNC();
        i = m_conf_cnc->init(m_szBasePath);
        if (CONF_iConfigured > 0) CONF_iConfigured = i;
    }
    return true;
}

bool sysConfig::saveBase() {
    FILE *fp = NULL;
    if (m_eType == DARK_SYS_TYPE::NONE) {
        PR_ERR("ERROR: Save - System is not initialized...\n");
        return false;
    }

    snprintf(m_szTemp, DARK_MAXPATH, "%s/conf.ifp", m_szBasePath);
    if (DARKPIF_fopen(&fp, m_szTemp, "w+") != true) {
        PR_ERR("ERROR: Save - Opening config file..[%s]\n", m_szTemp);
        return false;
    }

    fprintf(fp, "<HITEK_LINUX_UI>\n");
    if (m_eType == DARK_SYS_TYPE::MAIN_TX) fprintf(fp, "\t<type>MAIN_TX</type>\n");
    else if (m_eType == DARK_SYS_TYPE::REMOTE) fprintf(fp, "\t<type>REMOTE</type>\n");
    else fprintf(fp, "\t<type>INT_CNC</type>\n");
    fprintf(fp, "\t<status>%d</status>\n", CONF_iConfigured);
    if (strlen(CONF_szSysID) > 0) fprintf(fp, "\t<sys_id>%s</sys_id>\n", CONF_szSysID);
    fprintf(fp, "\t<setup_uri>%s</setup_uri>\n", CONF_szSetupURI);
    fprintf(fp, "\t<index_uri>%s</index_uri>\n", CONF_szIndexURI);
    fprintf(fp, "\t<network>\n");    
    fprintf(fp, "\t\t<broadcast_port>%d</broadcast_port>\n", CONF_net_iBroadcast);
    CONF_net.save(fp);
    fprintf(fp, "\t</network>\n");
    fprintf(fp, "</HITEK_LINUX_UI>\n");

    fclose(fp);
    return true;
}

bool sysConfig::getBaseSetupPayload(Json::Value &OUT_payload)
{
    Json::Value jsNet;

    if (m_eType == DARK_SYS_TYPE::NONE) return false;
    if (m_eType == DARK_SYS_TYPE::MAIN_TX) OUT_payload["sys_type"] = "MAIN_TX";
    else if (m_eType == DARK_SYS_TYPE::REMOTE) OUT_payload["sys_type"] = "REMOTE";
    else OUT_payload["sys_type"] = "INT_CNC";

    OUT_payload["status"] = CONF_iConfigured;
    if (strlen(CONF_szSysID) > 0) OUT_payload["sys_id"] = CONF_szSysID;
    else OUT_payload["sys_id"] = "NONE";

    jsNet["broadcast_port"] = CONF_net_iBroadcast;
    CONF_net.getPayload(jsNet);
    if(getIPAddress(m_szTemp, DARK_MAXPATH)) jsNet["cur_ip"] = m_szTemp;
    else jsNet["cur_ip"] = "NONE";

    OUT_payload["network"] = jsNet;
    return true;
}

bool sysConfig::setBaseSetupPayload(Json::Value payload) {
    Json::Value net = payload["network"];
    CONF_iConfigured = 1;
    DARK_STRCPY(CONF_szSysID, DARK_MIDSTR_S, payload["sys_id"].asCString());    
    CONF_net_iBroadcast = net["broadcast_port"].asInt();
    CONF_net.setPayload(net);

    return saveBase();    
}

bool sysConfig::getIPAddress(char *OUT_szIP, int iBufSize) {
    FILE *fp = NULL;
    snprintf(m_szTemp, DARK_MAXPATH, "sudo ifconfig %s | grep 'inet ' > %s/ip.txt", CONF_net.m_szInterface, m_szBasePath);
    if(system(m_szTemp) != 0) return false;
    snprintf(m_szTemp, DARK_MAXPATH, "%s/ip.txt", m_szBasePath);

    if (!DARKPIF_fopen(&fp, m_szTemp, "r")) return false;
    memset(OUT_szIP, 0, iBufSize);
    int i = 0;
    int iRes = 0;
    char c;
    //    inet 192.168.0.1 netmask xxx
    while(fread(&c, 1, 1, fp) > 0) {
        if (i == 0 || i == 2) { //시작, inet 다음 공백
            if (!(c == ' ' || c == '\t' || c == '\n' || c == '\r')) i++;
        }
        if (i == 1 || i == 3) { //inet, IP
            if (c == ' ' || c == '\t') i++;
            else {
                if (i == 3) {
                    OUT_szIP[iRes++] = c;
                    if (iRes >= iBufSize) break;
                }
            }
        }
        if (i >= 4) break;
    }    
    fclose(fp);
    if (iRes < 1) return false;
    return true;
}

bool sysConfig::getSetupPayload(Json::Value &OUT_payload) {
    switch(m_eType)
    {
    default:
    case DARK_SYS_TYPE::NONE:
        return false;
    case DARK_SYS_TYPE::MAIN_TX:
        if (m_conf_maintx != NULL) return m_conf_maintx->getSetupPayload(OUT_payload);
        break;
    case DARK_SYS_TYPE::REMOTE:
        if (m_conf_remote != NULL) return m_conf_remote->getSetupPayload(OUT_payload);
        break;
    case DARK_SYS_TYPE::INT_CNC:
        if (m_conf_cnc != NULL) return m_conf_cnc->getSetupPayload(OUT_payload);
        break;
    }

    return false;
}

bool sysConfig::setSetupPayload(Json::Value payload) {
    switch(m_eType)
    {
    default:
    case DARK_SYS_TYPE::NONE:
        return false;
    case DARK_SYS_TYPE::MAIN_TX:
        if (m_conf_maintx != NULL) return m_conf_maintx->setSetupPayload(payload);
        break;
    case DARK_SYS_TYPE::REMOTE:
        if (m_conf_remote != NULL) return m_conf_remote->setSetupPayload(payload);
        break;
    case DARK_SYS_TYPE::INT_CNC:
        if (m_conf_cnc != NULL) return m_conf_cnc->setSetupPayload(payload);
        break;
    }

    return false;
}

bool sysConfig::getLoginPayload(Json::Value &OUT_payload) {
    switch(m_eType)
    {
    default:
    case DARK_SYS_TYPE::NONE:
        return false;
    case DARK_SYS_TYPE::MAIN_TX:
        if (m_conf_maintx != NULL) return m_conf_maintx->getLoginPayload(OUT_payload);
        break;
    case DARK_SYS_TYPE::REMOTE:
        return false;
    case DARK_SYS_TYPE::INT_CNC:
        if (m_conf_cnc != NULL) return m_conf_cnc->getLoginPayload(OUT_payload);
        break;
    }

    return false;
}

bool sysConfig::setLoginPayload(Json::Value payload) {
    switch(m_eType)
    {
    default:
    case DARK_SYS_TYPE::NONE:
        return false;
    case DARK_SYS_TYPE::MAIN_TX:
        if (m_conf_maintx != NULL) return m_conf_maintx->setLoginPayload(payload);
        break;
    case DARK_SYS_TYPE::REMOTE:
        return false;
    case DARK_SYS_TYPE::INT_CNC:
        if (m_conf_cnc != NULL) return m_conf_cnc->setLoginPayload(payload);
        break;
    }

    return false;
}

const char *sysConfig::getLocalMediaPath() {
    switch(m_eType)
    {
    default:
    case DARK_SYS_TYPE::NONE:
    case DARK_SYS_TYPE::MAIN_TX:
    case DARK_SYS_TYPE::INT_CNC:
        break;
    case DARK_SYS_TYPE::REMOTE:
        if (m_conf_remote != NULL) return m_conf_remote->getLocalMediaPath();
    }

    return NULL;
}

pTAGInformation sysConfig::OpenConfig(Information_Parser &IFP) {
    switch(m_eType)
    {
    default:
    case DARK_SYS_TYPE::NONE:
    case DARK_SYS_TYPE::MAIN_TX:
        if (m_conf_maintx != NULL) return m_conf_maintx->OpenConfig(IFP);
        break;
    case DARK_SYS_TYPE::INT_CNC:
        if (m_conf_cnc != NULL) return m_conf_cnc->OpenConfig(IFP);
        break;
    case DARK_SYS_TYPE::REMOTE:
        if (m_conf_remote != NULL) return m_conf_remote->OpenConfig(IFP);        
    }

    return NULL;
}