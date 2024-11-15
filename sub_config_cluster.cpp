#include "sub_config_cluster.h"
#include "Information_Parser.h"

#define PR_ERR       printf

subConfigCluster::subConfigCluster() {
    m_iNum = 0;
    m_pIDs = NULL;
    m_pIPs = NULL;
}

subConfigCluster::~subConfigCluster() {
    resetValues();
}

void subConfigCluster::resetValues() {
    pStringChain pSCT;
    pStringChain pSC = m_pIDs;
    while(pSC != NULL) {
        pSCT = pSC;
        if (pSCT->szString != NULL) delete[] pSCT->szString;
        delete pSCT;
        pSC = pSC->pNext;
    }    
    pSC = m_pIPs;
    while(pSC != NULL) {
        pSCT = pSC;
        if (pSCT->szString != NULL) delete[] pSCT->szString;
        delete pSCT;
        pSC = pSC->pNext;
    }
    m_pIDs = NULL;
    m_pIPs = NULL;
    m_iNum = 0;
}

int subConfigCluster::init(void *pTag) {    
    Information_Parser IP;
    pTAGInformation pRoot, pT1, pT2;
    pStringChain pSC_id, pSC_ip;
    pStringChain pSC_id_pre, pSC_ip_pre;

    pRoot = IP.GetTag("clusters", (pTAGInformation)pTag);
    if (pRoot == NULL) return 100;

    pSC_id_pre = NULL;
    pSC_ip_pre = NULL;

    pT1 = IP.GetTag("cluster", pRoot);
    while(pT1 != NULL) {
        if((pT2 = IP.GetTag("ID", pT1)) == NULL) return 0;
        pSC_id = new StringChain;
        pSC_id->pNext = NULL;
        pSC_id->iLen = DARK_MIDSTR_S;
        pSC_id->szString = new char[DARK_MIDSTR_S];
        DARK_STRCPY(pSC_id->szString, DARK_MIDSTR_S, (char *)pT2->pBuffer);
        if (pSC_id_pre == NULL) m_pIDs = pSC_id;            
        else pSC_id_pre->pNext = pSC_id;
        pSC_id_pre = pSC_id;
        
        if((pT2 = IP.GetTag("IP", pT1)) == NULL) return 0;
        pSC_ip = new StringChain;
        pSC_ip->pNext = NULL;
        pSC_ip->iLen = DARK_MIDSTR_S;
        pSC_ip->szString = new char[DARK_MIDSTR_S];
        DARK_STRCPY(pSC_ip->szString, DARK_MIDSTR_S, (char *)pT2->pBuffer);
        if (pSC_ip_pre == NULL) m_pIPs = pSC_ip;            
        else pSC_ip_pre->pNext = pSC_ip;
        pSC_ip_pre = pSC_ip;

        m_iNum++;
        pT1 = pT1->pNext;
    }
    
    return 100;
}

bool subConfigCluster::save(FILE *fp) {
    pStringChain pSC_id, pSC_ip;

    if (m_iNum < 1) return true;
    pSC_id = m_pIDs;
    pSC_ip = m_pIPs;
    fprintf(fp, "\t\t<clusters>\n");    
    for(int i = 0; i < m_iNum; i++) {
        if (pSC_id == NULL || pSC_ip == NULL) return false;
        fprintf(fp, "\t\t\t<cluster>\n");
        fprintf(fp, "\t\t\t\t<ID>%s</ID>\n", pSC_id->szString);
        pSC_id = pSC_id->pNext;
        fprintf(fp, "\t\t\t\t<IP>%s</IP>\n", pSC_ip->szString);
        pSC_ip = pSC_ip->pNext;
        fprintf(fp, "\t\t\t</cluster>\n");
    }
    fprintf(fp, "\t\t</clusters>\n");
    return true;
}

bool subConfigCluster::getPayload(Json::Value &OUT_payload) {        
    pStringChain pSC_id, pSC_ip;

    if (m_iNum < 1) return true;
    pSC_id = m_pIDs;
    pSC_ip = m_pIPs;
    
    Json::Value clusters;
    for(int i = 0; i < m_iNum; i++) {
        Json::Value cluster;        
        cluster["id"] = pSC_id->szString;
        pSC_id = pSC_id->pNext;
        cluster["ip"] = pSC_ip->szString;
        pSC_ip = pSC_ip->pNext;
        clusters.append(cluster);
    }
    //Json::StyledWriter w;
    //printf(w.write(clusters).c_str());

    OUT_payload["clusters"] = clusters;
    return true;
}

bool subConfigCluster::setPayload(Json::Value payload) {    
    resetValues();
    if (!payload.isMember("clusters")) return true;
    
    pStringChain pSC_id, pSC_ip;
    pStringChain pSC_id_pre, pSC_ip_pre;
    Json::Value clusters = payload["clusters"];
    m_iNum = clusters.size();
    pSC_id_pre = NULL;
    pSC_ip_pre = NULL;
    for(int i = 0; i < m_iNum; i++) {
        pSC_id = new StringChain;
        pSC_id->pNext = NULL;
        pSC_id->iLen = DARK_MIDSTR_S;
        pSC_id->szString = new char[DARK_MIDSTR_S];
        DARK_STRCPY(pSC_id->szString, DARK_MIDSTR_S, clusters[i]["id"].asCString());
        if (pSC_id_pre == NULL) m_pIDs = pSC_id;            
        else pSC_id_pre->pNext = pSC_id;
        pSC_id_pre = pSC_id;
        
        pSC_ip = new StringChain;
        pSC_ip->pNext = NULL;
        pSC_ip->iLen = DARK_MIDSTR_S;
        pSC_ip->szString = new char[DARK_MIDSTR_S];
        DARK_STRCPY(pSC_ip->szString, DARK_MIDSTR_S, clusters[i]["ip"].asCString());
        if (pSC_ip_pre == NULL) m_pIPs = pSC_ip;            
        else pSC_ip_pre->pNext = pSC_ip;
        pSC_ip_pre = pSC_ip;
    }
    return true;
}

