#pragma once
#include "common_inc.h"

class localMedia {
public:
    localMedia();
    ~localMedia();

    int init(const char *szBasePath, const char *szScriptPath = NULL);
    bool save();
    bool getMediaPayload(Json::Value &OUT_payload);
    bool addLocalMedia(Json::Value payload);
    bool deleteLocalMedia(Json::Value payload);

protected:
    const char *m_cszBasePath;
    char *m_szScriptPath;
    char *m_szTemp;

    uint32_t m_media_seq;
    Json::Value m_media_list;
};
