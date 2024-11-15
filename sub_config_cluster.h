#pragma once
#include "common_inc.h"

class subConfigCluster {
public:
    subConfigCluster();
    ~subConfigCluster();

    int init(void *pTag);
    bool save(FILE *fp);
    bool getPayload(Json::Value &OUT_payload);
    bool setPayload(Json::Value payload);

protected:
    int m_iNum;
    pStringChain m_pIDs;
    pStringChain m_pIPs;

    void resetValues();
};
