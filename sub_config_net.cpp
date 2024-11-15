#include "sub_config_net.h"
#include "Information_Parser.h"

#define PR_ERR       printf

subConfigNet::subConfigNet() {
    m_bIsStaticIP = false;
    m_szIP = NULL;
    m_szNetmask = NULL;
    m_szGateway = NULL;
    m_szDNS1 = NULL;
    m_szDNS2 = NULL;
    
    m_szInterface = new char[DARK_MIDSTR_S];
    snprintf(m_szInterface, DARK_MIDSTR_S, "eth0");
}

subConfigNet::~subConfigNet() {
    if (m_szIP != NULL) delete[] m_szIP;
    if (m_szInterface != NULL) delete[] m_szInterface;
    if (m_szNetmask != NULL) delete[] m_szNetmask;
    if (m_szGateway != NULL) delete[] m_szGateway;
    if (m_szDNS1 != NULL) delete[] m_szDNS1;
    if (m_szDNS2 != NULL) delete[] m_szDNS2;
}

int subConfigNet::init(void *pTag) {    
    Information_Parser IP;
    pTAGInformation pT1, pT2;

    if ((pT1 = IP.GetTag("interface", (pTAGInformation)pTag)) != NULL) snprintf(m_szInterface, DARK_MIDSTR_S, "%s", pT1->pBuffer);    
    if ((pT1 = IP.GetTag("v4type", (pTAGInformation)pTag)) != NULL) {
        if (strcmp((char *)pT1->pBuffer, "STATIC") == 0) {
            if ((pT1 = IP.GetTag("static_info", (pTAGInformation)pTag)) != NULL) {
                m_bIsStaticIP = true;
                if (m_szIP == NULL) m_szIP = new char[DARK_MIDSTR_S];
                if (m_szNetmask == NULL) m_szNetmask = new char[DARK_MIDSTR_S];
                if (m_szGateway == NULL) m_szGateway = new char[DARK_MIDSTR_S];
                if (m_szDNS1 == NULL) m_szDNS1 = new char[DARK_MIDSTR_S];
                if (m_szDNS2 == NULL) m_szDNS2 = new char[DARK_MIDSTR_S];
                m_szIP[0] = 0;
                m_szNetmask[0] = 0;
                m_szGateway[0] = 0;
                m_szDNS1[0] = 0;
                m_szDNS2[0] = 0;
                if ((pT2 = IP.GetTag("ip", pT1)) != NULL) snprintf(m_szIP, DARK_MIDSTR_S, "%s", pT2->pBuffer);
                if ((pT2 = IP.GetTag("netmask", pT1)) != NULL) snprintf(m_szNetmask, DARK_MIDSTR_S, "%s", pT2->pBuffer);
                if ((pT2 = IP.GetTag("gateway", pT1)) != NULL) snprintf(m_szGateway, DARK_MIDSTR_S, "%s", pT2->pBuffer);
                if ((pT2 = IP.GetTag("dns1", pT1)) != NULL) snprintf(m_szDNS1, DARK_MIDSTR_S, "%s", pT2->pBuffer);
                if ((pT2 = IP.GetTag("dns2", pT1)) != NULL) snprintf(m_szDNS2, DARK_MIDSTR_S, "%s", pT2->pBuffer);
            }
            else m_bIsStaticIP = false;
        }
        else m_bIsStaticIP = false;
    }
    
    return 100;
}

bool subConfigNet::save(FILE *fp, int iTap) {
    char *szTap = new char[iTap + 1];
    for(int i = 0; i < iTap; i++) szTap[i] = '\t';
    szTap[iTap] = 0;

    fprintf(fp, "%s<interface>%s</interface>\n", szTap, m_szInterface);    
    if (m_bIsStaticIP) {
        fprintf(fp, "%s<v4type>STATIC</v4type>\n", szTap);
        fprintf(fp, "%s<static_info>\n", szTap);
        fprintf(fp, "%s\t<ip>%s</ip>\n", szTap, m_szIP);
        fprintf(fp, "%s\t<netmask>%s</netmask>\n", szTap, m_szNetmask);
        fprintf(fp, "%s\t<gateway>%s</gateway>\n", szTap, m_szGateway);
        if (strlen(m_szDNS1) > 0) fprintf(fp, "%s\t<dns1>%s</dns1>\n", szTap, m_szDNS1);
        if (strlen(m_szDNS2) > 0) fprintf(fp, "%s\t<dns2>%s</dns2>\n", szTap, m_szDNS2);
        fprintf(fp, "%s</static_info>\n", szTap);
    }
    else fprintf(fp, "%s<v4type>DHCP</v4type>\n", szTap);
    delete[] szTap;
    return true;
}

bool subConfigNet::getPayload(Json::Value &OUT_payload) {        
    OUT_payload["interface"] = m_szInterface;    
    if (m_bIsStaticIP) {
        OUT_payload["type"] = "STATIC";
        OUT_payload["static_ip"] = m_szIP;
        OUT_payload["static_netmask"] = m_szNetmask;
        OUT_payload["static_gateway"] = m_szGateway;
        OUT_payload["static_dns1"] = m_szDNS1;
        OUT_payload["static_dns2"] = m_szDNS2;
    }
    else OUT_payload["type"] = "DHCP";
    
    return true;
}

bool subConfigNet::setPayload(Json::Value payload) { 
    if (!payload.isMember("interface")) return false;
    DARK_STRCPY(m_szInterface, DARK_MIDSTR_S, payload["interface"].asCString());    
    if (strcmp(payload["type"].asCString(), "STATIC") == 0) {
        m_bIsStaticIP = true;
        if (m_szIP == NULL) m_szIP = new char[DARK_MIDSTR_S];
        if (m_szNetmask == NULL) m_szNetmask = new char[DARK_MIDSTR_S];
        if (m_szGateway == NULL) m_szGateway = new char[DARK_MIDSTR_S];
        if (m_szDNS1 == NULL) m_szDNS1 = new char[DARK_MIDSTR_S];
        if (m_szDNS2 == NULL) m_szDNS2 = new char[DARK_MIDSTR_S];
        m_szIP[0] = 0;
        m_szNetmask[0] = 0;
        m_szGateway[0] = 0;
        m_szDNS1[0] = 0;
        m_szDNS2[0] = 0;
        DARK_STRCPY(m_szIP, DARK_MIDSTR_S, payload["static_ip"].asCString());        
        DARK_STRCPY(m_szNetmask, DARK_MIDSTR_S, payload["static_netmask"].asCString());        
        DARK_STRCPY(m_szGateway, DARK_MIDSTR_S, payload["static_gateway"].asCString());        
        if (payload.isMember("static_dns1")) DARK_STRCPY(m_szDNS1, DARK_MIDSTR_S, payload["static_dns1"].asCString());        
        if (payload.isMember("static_dns2")) DARK_STRCPY(m_szDNS2, DARK_MIDSTR_S, payload["static_dns2"].asCString());        
    }
    else m_bIsStaticIP = false;

    if (!_sys_setup_netif(!m_bIsStaticIP, m_szInterface, m_szIP, m_szNetmask, m_szGateway, m_szDNS1, m_szDNS2)) return false;
    return true;
}

bool subConfigNet::_sys_setup_netif(bool bDHCP, const char *szIf, const char *szIP, const char *szMask, const char *szGateway, const char *szDNS1, const char *szDNS2)
{
    if (szIf == NULL) return false;

    FILE *fp = NULL;
    char *szTemp = new char[DARK_MAXPATH];
    if (szTemp == NULL) {
        PR_ERR("ERROR: Not enough memory...\n");
        return false;
    }

    if (access("/HitekDPA/scripts/setup_netif.sh", X_OK) == 0) {
        if (bDHCP) snprintf(szTemp, DARK_MAXPATH, "/HitekDPA/scripts/setup_netif.sh dhcp %s", szIf);
        else {
            snprintf(szTemp, DARK_MAXPATH, "/HitekDPA/scripts/setup_netif.sh static %s %s %s %s", szIf, szIP, szMask, szGateway);
            if (szDNS1 != NULL && strlen(szDNS1) > 0) snprintf(szTemp + strlen(szTemp), DARK_MAXPATH - strlen(szTemp), " %s", szDNS1);
            if (szDNS2 != NULL && strlen(szDNS2) > 0) snprintf(szTemp + strlen(szTemp), DARK_MAXPATH - strlen(szTemp), " %s", szDNS2);
        }
        printf("Run command: %s\n", szTemp);
        system(szTemp);
        delete[] szTemp;
        return true;
    }

    snprintf(szTemp, DARK_MAXPATH, "/etc/network/interfaces.d/hitekconf_%s", szIf);
    if (!DARKPIF_fopen(&fp, szTemp, "w+")) {
        PR_ERR("ERROR: File open failed: [%s]\n", szTemp);
        delete[] szTemp;
        return false;
    }
    delete[] szTemp;
    fprintf(fp, "auto %s\n", szIf);
    if (bDHCP) {
        fprintf(fp, "iface %s inet dhcp\n", szIf);
    }
    else {
        fprintf(fp, "iface %s inet static\n", szIf);
        fprintf(fp, "address %s\n", szIP);
        fprintf(fp, "netmask %s\n", szMask);
        fprintf(fp, "gateway %s\n", szGateway);
        if (szDNS1 != NULL && strlen(szDNS1) > 0) fprintf(fp, "dns-nameserver %s\n", szDNS1);
        if (szDNS2 != NULL && strlen(szDNS2) > 0) fprintf(fp, "dns-nameserver %s\n", szDNS2);
    }

    fclose(fp);   
    return true;
}
