#include "local_media.h"

#define PR_ERR       printf
#define PR_DBG       printf

localMedia::localMedia() {
    m_cszBasePath = NULL;
    m_szScriptPath = NULL;
    m_szTemp = new char[DARK_MAXPATH];
    m_szTemp[0] = 0;

    m_media_seq = 0;    
}

localMedia::~localMedia() {
    if (m_szTemp != NULL) delete[] m_szTemp;
    if (m_szScriptPath != NULL) delete[] m_szScriptPath;
}

int localMedia::init(const char *szBasePath, const char *szScriptPath) {    
    m_cszBasePath = szBasePath;    
    if (m_szScriptPath != NULL) delete[] m_szScriptPath;
    m_szScriptPath = new char[DARK_MAXPATH];
    if (szScriptPath == NULL) snprintf(m_szScriptPath, DARK_MAXPATH, "/HitekDPA/scripts");
    else strcpy(m_szScriptPath, szScriptPath);

    Json::Reader r;
    FILE *fp = NULL;

    snprintf(m_szTemp, DARK_MAXPATH, "%s/media_list.json", m_cszBasePath);
    if (DARKPIF_fopen(&fp, m_szTemp, "r") != true) {
        PR_DBG("Media list file opening failed..[%s]\n", m_szTemp);
        return 0;
    }

    fseek(fp, 0, SEEK_END);
    size_t len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (len < 1) {
        PR_ERR("ERROR: Media list file length: %d\n", len);
        fclose(fp);
        return -1;
    }

    char *szT = new char[len + 1];
    if (szT == NULL) {
        PR_ERR("ERROR: Memory allocation..: %d\n", len);
        fclose(fp);
        return -2;
    }

    fread(szT, 1, len, fp);
    fclose(fp);
    szT[len] = 0;

    Json::Value tList;
    int i;
    DarkString dsT;
    bool bRes = r.parse(szT, tList);
    delete[] szT;

	if (!bRes) {
		PR_ERR("ERROR: Media list - JSON Parsing failed");		
		return -3;
	}

    //SEQ검사
    for(i = 0; i < tList.size(); i++) {
        if (tList[i]["seq"].asUInt() > m_media_seq) m_media_seq = tList[i]["seq"].asUInt() + 1;
    }

    m_media_list.clear();
    //실제로 파일이 있는지 검사
    for(i = 0; i < tList.size(); i++) {
        dsT.SetString(tList[i]["local_uri"].asCString());
        if (access(dsT.m_szString, 04) != 0) { //04 = 읽기 권한
            PR_ERR("MediaList init error: File is not exist - [%s]\n", dsT.m_szString);
        }
        else m_media_list.append(tList[i]);
    }

    return 0;
}

bool localMedia::save() {
    FILE *fp = NULL;

    snprintf(m_szTemp, DARK_MAXPATH, "%s/media_list.json", m_cszBasePath);
    if (DARKPIF_fopen(&fp, m_szTemp, "w+") != true) {
        PR_ERR("ERROR: Save - Opening list file..[%s]\n", m_szTemp);
        return false;
    }

    DarkString ds;
    Json::FastWriter w;

    ds.SetString(w.write(m_media_list).c_str());    
    if (ds.m_uiSize < 1) fprintf(fp, "[]\n");
    else fwrite(ds.m_szString, 1, ds.m_uiSize, fp);

    fclose(fp);
    return true;
}

bool localMedia::getMediaPayload(Json::Value &OUT_payload) {    
    OUT_payload = m_media_list;
    return true;
}

bool localMedia::addLocalMedia(Json::Value payload) {        
    if(!payload.isMember("mtx_ip") || !payload.isMember("mtx_uuid")) {
        PR_ERR("Invalid JSON: No MTX Info\n");
        return false;
    }
    if(!payload.isMember("remote_uri") || !payload.isMember("remote_idx") || !payload.isMember("title")) {
        PR_ERR("Invalid JSON: No Media Info\n");
        return false;
    }

    //리스트 중복 검색
    int i;
    DarkString dsT, dsT2;

    dsT.SetString(payload["mtx_uuid"].asCString());
    dsT2.SetString(payload["remote_uri"].asCString());
    for (i = 0; i < m_media_list.size(); i++) {
        if (strcmp(dsT.m_szString, m_media_list[i]["mtx_uuid"].asCString()) == 0
            && strcmp(dsT2.m_szString, m_media_list[i]["remote_uri"].asCString()) == 0) 
        {
            PR_DBG("addLocalMedia: Duplicate media exist already..\n");
            return false;
        }
    }

    //파일 확장자 찾기 및 이름 생성
    char szT[255];
        
    for (i = dsT2.m_uiSize - 1; i >= 0; i--) {
        if (dsT2.m_szString[i] == '.') break;
    }
    snprintf(szT, 255, "%08d.%s", m_media_seq++, dsT2.m_szString + i + 1);

    snprintf(m_szTemp, DARK_MAXPATH, "export CALLED_IP=%s; %s/media_downloader.sh %s %s %s"
        , payload["mtx_ip"].asCString()
        , m_szScriptPath
        , payload["remote_uri"].asCString()
        , m_cszBasePath, szT);

    system(m_szTemp);

    //파일이 제대로 받아 졌는지 검사
    snprintf(m_szTemp, DARK_MAXPATH, "%s/%s", m_cszBasePath, szT);
    if (access(m_szTemp, 04) != 0) { //04 = 읽기 권한
        PR_ERR("addLocalMedia: File download failed [%s]\n", m_szTemp);
        return false;
    }

    Json::Value nM;
    nM["mtx_uuid"] = payload["mtx_uuid"];
    nM["remote_uri"] = payload["remote_uri"];
    nM["remote_idx"] = payload["remote_idx"];
    nM["seq"] = m_media_seq - 1;
    nM["local_uri"] = m_szTemp;
    nM["title"] = payload["title"];
    m_media_list.append(nM);

    return save();
}

bool localMedia::deleteLocalMedia(Json::Value payload) {        
    if(!payload.isMember("seq")) {
        PR_DBG("deleteLocalMedia: Invalid JSON..\n");
        return false;
    }
    int iTarget = -1;
    int i;
    for (i = 0; i < m_media_list.size(); i++) {
        if (m_media_list[i]["seq"].asUInt() == payload["seq"].asUInt()) {
            iTarget = i;
            break;
        }
    }
    if (iTarget < 0) {
        PR_DBG("deleteLocalMedia: No target..\n");
        return false;
    }

    Json::Value newList;
    for (i = 0; i < m_media_list.size(); i++) {
        if (i == iTarget) continue;
        newList.append(m_media_list[i]);
    }

    m_media_list.clear();
    m_media_list = newList;

    return save();
}
