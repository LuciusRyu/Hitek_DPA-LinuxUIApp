#include "MainLogic.h"

static void printHelp() {
    g_print("Hitek UI version 1.0\n");
    g_print("Available option list\n");
    g_print("\t--help \tShow help\n");
    g_print("\t--target [URL] \tSpecific target page\n");
    g_print("\t--console \tEnable console debug log\n");
}

MainLogic::MainLogic()
{
    m_iSysState = 0;
    m_pApp = NULL;
    m_pWindow = NULL;    
    m_pVbox = NULL;
    m_pWebview = NULL;

    m_bStarted = false;
    
    m_szScriptBuf = new char[DARK_MAXPATH];
    m_iScriptBuf = DARK_MAXPATH;
    m_szLastURI = new char[DARK_MAXPATH];
    m_szLastURI[0] = 0;
    m_net_detector = NULL;
    m_local_media = NULL;
    m_i2c = NULL;
    m_pGPIO = NULL;
    m_bShowInspector = false;
    m_bScriptCallable = false;

    DARKCRITICAL_INIT(CRITICAL_script);
	pthread_attr_init(&m_THRATT_detach);
	pthread_attr_setdetachstate(&m_THRATT_detach, PTHREAD_CREATE_DETACHED);
}

MainLogic::~MainLogic()
{
    if (m_pWebview != NULL) g_object_unref(m_pWebview);
    if (m_pVbox != NULL) g_object_unref(m_pVbox);
    if (m_pWindow != NULL) g_object_unref(m_pWindow);
    if (m_pApp != NULL) g_object_unref(m_pApp);    
    if (m_szScriptBuf != NULL) delete[] m_szScriptBuf;    
    if (m_szLastURI != NULL) delete[] m_szLastURI;
    if (m_net_detector != NULL) delete m_net_detector;
    if (m_local_media != NULL) delete m_local_media;
    if (m_i2c != NULL) delete m_i2c;
    if (m_pGPIO != NULL) delete m_pGPIO;

    DARKCRITICAL_DELETE(CRITICAL_script);
}

int MainLogic::StartSystem(int argc, char** argv)
{
    int ret = 0;    
    pthread_t pThr;
    char *szConfigPath = NULL;

    if (m_pApp != NULL) return -1;

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--configDir") == 0) {            
            if (argc > i + 1) {
                szConfigPath = argv[i + 1];
                g_print("Config path set to %s\n", szConfigPath);
                i++;
            }
        }
        else if (strcmp(argv[i], "--debugger") == 0) {     
            m_bShowInspector = true;
            g_print("Inspector option enabled\n");
            i++;       
        }
    }

    if (m_config.init(szConfigPath) != true) return -2;
    if (m_config.sysType() == DARK_SYS_TYPE::REMOTE) {
        g_print("Do initialization for Remote\n");
        m_local_media = new localMedia();
        if (m_local_media->init(m_config.getLocalMediaPath()) != 0) {
            g_print("ERROR: Local media init failed..");
            return -2;
        }
        m_pGPIO = new GPIO_Control();
        m_i2c = new I2C_Interface();
        if (m_i2c->Init(szConfigPath, m_pGPIO) != 0) {
            g_print("ERROR: I2C Interface init failed..");
            return -3;
        }
        m_net_detector = new netDetector(on_NetState, this);
        if (m_net_detector->init((unsigned int)m_config.getNetPort()) != 0) {
            g_print("ERROR: Net detector init failed..");
            return -1;
        }
    }

    m_pApp = gtk_application_new(HITEK_APP_ID, G_APPLICATION_HANDLES_COMMAND_LINE);
    g_signal_connect(m_pApp, "command-line", G_CALLBACK(cmdlineCallback), this);

    m_bStarted = true;
    m_iSysState = 1;
    pthread_create(&pThr, &m_THRATT_detach, THR_Loop, this);

    ret = g_application_run(G_APPLICATION(m_pApp), argc, argv);
    //m_bStarted = false;
    return ret;
}

int MainLogic::cmdlineCallback(GtkApplication *app, GApplicationCommandLine *cmdline, gpointer data) {
    MainLogic *pP = (MainLogic *)data;
    return pP->_cmdlineCallback(cmdline);
}

int MainLogic::_cmdlineCallback(GApplicationCommandLine *cmdline)
{
    gchar **argv;
    gint argc;
    gint i;
    bool bConsoleLog = false;
    bool bIsHelp = false;    
    WebKitHardwareAccelerationPolicy eGPUAcc = WEBKIT_HARDWARE_ACCELERATION_POLICY_ALWAYS;

    argv = g_application_command_line_get_arguments(cmdline, &argc);

    for (i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--console") == 0) bConsoleLog = true;
        else if (strcmp(argv[i], "--help") == 0) bIsHelp = true;
        else if (strcmp(argv[i], "--gpu_acc") == 0) {
            if (argc > i + 1) {                
                i++;
                if (strcmp(argv[i], "on_demand") == 0) eGPUAcc = WEBKIT_HARDWARE_ACCELERATION_POLICY_ON_DEMAND;
                else if (strcmp(argv[i], "always") == 0) eGPUAcc = WEBKIT_HARDWARE_ACCELERATION_POLICY_ALWAYS;
                else if (strcmp(argv[i], "never") == 0) eGPUAcc = WEBKIT_HARDWARE_ACCELERATION_POLICY_NEVER;
                else g_print("ERROR: Unknown GPU Policy: [on_demand:always:never]\n");
            }            
        }
    }

    g_strfreev(argv);

    if (bIsHelp) {
        printHelp();
        m_iSysState = 0;
        return 0;
    }

    prepareWindow();
    webkitSetup(eGPUAcc);
    if (m_config.IsConfigured()) DARK_STRCPY(m_szLastURI, DARK_MAXPATH, m_config.getIndexURI());
    else DARK_STRCPY(m_szLastURI, DARK_MAXPATH, m_config.getSetupURI());
    openURI(m_szLastURI);    
    return 0;
}

void *MainLogic::THR_Loop(void *pParam)
{
    MainLogic *pP = (MainLogic *)pParam;
    pP->_THR_Loop();   
    return NULL;
}

void MainLogic::_THR_Loop()
{
    unsigned int nCnt = 0;

    while(m_iSysState > 0) {
        if (m_iSysState == 2) {
            g_print("Page load faile detected... start retry count\n");
            m_iSysState = 3;
            nCnt = 0;            
        }
        else if(m_iSysState == 3) {
            nCnt++;
            if (nCnt >= 30) {
                m_iSysState = 1;
                g_print("Retry to open page..\n");                
                g_idle_add(update_gui, this);
            }
        }
        DARK_mSleep(100);
    }
    m_bStarted = false;
}

void MainLogic::prepareWindow()
{
	g_assert(GTK_IS_APPLICATION(m_pApp));

	m_pWindow = gtk_application_get_active_window(m_pApp);
	if (m_pWindow == NULL) {
		//window = (GtkWindow *)g_object_new(GTK_TYPE_WINDOW, "application", app, "default-width", 1278, "default-height", 798, NULL);
        m_pWindow = (GtkWindow *)gtk_application_window_new(m_pApp);
        gtk_window_set_title(GTK_WINDOW(m_pWindow), "HitekUI");
        gtk_window_fullscreen(GTK_WINDOW(m_pWindow));
	}
	gtk_window_set_decorated(m_pWindow, FALSE);
    
    //m_pWebview = webkit_web_view_new();
    g_autoptr(WebKitUserContentManager) content_manager = create_scriptCallback_mng(this);
    m_pWebview = webkit_web_view_new_with_user_content_manager(content_manager);
    
    g_signal_connect(m_pWebview, "load-failed", G_CALLBACK (on_webview_load_failed), this);

	m_pVbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(m_pVbox),GTK_WIDGET(m_pWebview),TRUE,TRUE, 0);
    gtk_container_add(GTK_CONTAINER(m_pWindow),GTK_WIDGET(m_pVbox));
    gtk_widget_show_all(GTK_WIDGET(m_pVbox));
    gtk_window_present(m_pWindow);	
}

void MainLogic::openURI(const char *szURI, bool bReload)
{
    if (bReload) {
        g_print("Reload page: [%s]\n", szURI);
        webkit_web_view_reload_bypass_cache(WEBKIT_WEB_VIEW(m_pWebview));
    }
    else {
        g_print("Open page: [%s]\n", szURI);
	    webkit_web_view_load_uri(WEBKIT_WEB_VIEW(m_pWebview), szURI);
        if (m_bShowInspector) {
            g_print("Show Inspector!\n");
            WebKitWebInspector *inspector = webkit_web_view_get_inspector(WEBKIT_WEB_VIEW(m_pWebview));
            webkit_web_inspector_show (WEBKIT_WEB_INSPECTOR(inspector));            
        }
    }
}

gboolean MainLogic::update_gui(gpointer data) 
{
    MainLogic *pP = (MainLogic *)data;
    return pP->_update_gui();
}

gboolean MainLogic::_update_gui() 
{
    openURI(m_szLastURI, true);
    return FALSE;
}

gboolean MainLogic::on_webview_load_failed(WebKitWebView  *webview, WebKitLoadEvent load_event, gchar *failing_uri, GError *error, gpointer user_data)
{
    MainLogic *pP = (MainLogic *)user_data;
    return pP->_on_webview_load_failed(webview, load_event, failing_uri, error);
}

gboolean MainLogic::_on_webview_load_failed(WebKitWebView  *webview, WebKitLoadEvent load_event, gchar *failing_uri, GError *error)
{
    g_printerr("_on_webview_load_failed: %s: %s\n",failing_uri,error->message);
    m_iSysState = 2;
    return FALSE;
}

void MainLogic::webkitSetup(WebKitHardwareAccelerationPolicy eGPUAcc) {
    WebKitSettings *pSet = webkit_web_view_get_settings(WEBKIT_WEB_VIEW(m_pWebview));
    if (webkit_settings_get_enable_write_console_messages_to_stdout(pSet)) g_print("Console log enabled\n");
    else g_print("Console log diabled\n");
    webkit_settings_set_enable_write_console_messages_to_stdout(pSet, true);

    webkit_settings_set_allow_file_access_from_file_urls(pSet, true); //로컬 파일 허용
    webkit_settings_set_allow_universal_access_from_file_urls(pSet, true); //로컬파일 CORS허용
    webkit_settings_set_enable_offline_web_application_cache(pSet, false); //로컬캐시 제거
    if (m_bShowInspector) webkit_settings_set_enable_developer_extras (pSet, true);    
    webkit_settings_set_hardware_acceleration_policy(pSet, eGPUAcc);
}

WebKitUserContentManager* MainLogic::create_scriptCallback_mng(gpointer user_data)
{
    g_autoptr(WebKitUserContentManager) content_manager = webkit_user_content_manager_new();
    webkit_user_content_manager_register_script_message_handler(content_manager, "hitek_native");
    g_signal_connect(content_manager, "script-message-received::hitek_native", G_CALLBACK(script_callback_function), user_data);
    return (WebKitUserContentManager *)g_steal_pointer(&content_manager);
}

void MainLogic::script_callback_function(WebKitWebView *web_view, WebKitJavascriptResult *js_result, gpointer user_data) {
    MainLogic *pP = (MainLogic *)user_data;
    pP->_script_callback_function(web_view, js_result);
}

void MainLogic::_script_callback_function(WebKitWebView *web_view, WebKitJavascriptResult *js_result) {
    JSCValue *value = webkit_javascript_result_get_js_value(js_result);
    if (!jsc_value_is_string(value)) {
        g_warning("Invalid Hitek native message: argument is not a string");
        return;
    }

	Json::Reader r;
	Json::Value jsv;
    char szAct[DARK_MIDSTR_S];
    szAct[0] = 0;

    g_autofree char *value_as_string = jsc_value_to_string(value);

    m_bScriptCallable = true; //어쨌든 웹에서 호출 한거니까..

    //g_print("Script call back!! msg=%s\n", value_as_string);
	if (!r.parse(value_as_string, jsv)) {
		g_warning("_script_callback_function - JSON Parsing failed");
		g_warning("RECV STR:\n%s", value_as_string);
		return;
	}

	if (!jsv.isMember("act") || !jsv.isMember("payload")) {
        g_warning("_script_callback_function - No ACT, Payload");
        return;
    }
	if (!jsv["act"].isString()) {
		g_warning("_script_callback_function - Invalid ACT");
		return;
	}

    DARK_STRCPY(szAct, DARK_MIDSTR_S, jsv["act"].asCString());    
    on_script_request(szAct, jsv["payload"]);    
}


void MainLogic::on_javascript_finished (GObject *object, GAsyncResult *result, gpointer user_data) 
{
    JSCValue               *value;
    GError                 *error = NULL;
    MainLogic *pP = (MainLogic *)user_data;

    value = webkit_web_view_evaluate_javascript_finish (WEBKIT_WEB_VIEW(object), result, &error);    
    if (!value) {
        g_warning ("Error running javascript-1: %s", error->message);
        g_error_free (error);
        //printf("Script unlocked\n");
        DARKCRITICAL_UNLOCK(pP->CRITICAL_script);
        return;
    }

    if (jsc_value_is_string (value)) {
        JSCException *exception;
        gchar        *str_value;

        str_value = jsc_value_to_string (value);
        exception = jsc_context_get_exception (jsc_value_get_context (value));
        if (exception) g_warning ("Error running javascript-2: %s", jsc_exception_get_message (exception));
        //else g_print ("Script result: %s\n", str_value);
        g_free (str_value);
    } else {
        g_warning ("Error running javascript-3: unexpected return value");
    }
    //printf("Script unlocked\n");
    DARKCRITICAL_UNLOCK(pP->CRITICAL_script);
}

void MainLogic::callScript(Json::Value &jsVal) {    
    DARKCRITICAL_LOCK(CRITICAL_script);
    //printf("Script locked!!!\n");
    //Json::StyledWriter w;
    Json::FastWriter w;

    int iLen = strlen(w.write(jsVal).c_str());    
    if (iLen + 64 > m_iScriptBuf) {
        delete[] m_szScriptBuf;
        m_iScriptBuf = iLen + m_iScriptBuf;
        m_szScriptBuf = new char[m_iScriptBuf];
    }
    g_snprintf(m_szScriptBuf, m_iScriptBuf, "callback_from_native(`\n%s`);", w.write(jsVal).c_str()); 
    //g_print("%s\n", m_szScriptBuf);
    webkit_web_view_evaluate_javascript(WEBKIT_WEB_VIEW(m_pWebview), m_szScriptBuf, -1, NULL, NULL, NULL, on_javascript_finished, this);
}

void MainLogic::on_script_request(const char *szAct, Json::Value jsPayload) 
{
    Json::Value jsV, jsP;
    if (strcmp(szAct, "SETUP_START") == 0) {
        m_config.getBaseSetupPayload(jsP);   
        jsV["act"] = "SETUP_START";
        jsV["payload"] = jsP;

        callScript(jsV);
    }
    else if (strcmp(szAct, "SYS_REBOOT") == 0) {
        system("sudo reboot");
    }
    else if (strcmp(szAct, "SYS_OPEN_SETUP") == 0) {
        if (strcmp(m_config.getSetupURI(), m_szLastURI) == 0) openURI(m_szLastURI, true);
        else {
            DARK_STRCPY(m_szLastURI, DARK_MAXPATH, m_config.getSetupURI());
            openURI(m_szLastURI);
        }
    }
    else if (strcmp(szAct, "SYS_OPEN_MAIN") == 0) {
        if (strcmp(m_config.getIndexURI(), m_szLastURI) == 0) openURI(m_szLastURI, true);
        else {
            DARK_STRCPY(m_szLastURI, DARK_MAXPATH, m_config.getIndexURI());
            openURI(m_szLastURI);
        }
    }
    else if (strcmp(szAct, "SYS_RUN_COMMAND") == 0) {
        runSystemCommand(jsPayload);
    }
    else if (strcmp(szAct, "GET_BASEINFO") == 0) {
        m_config.getBaseSetupPayload(jsP);   
        jsV["act"] = "GET_BASEINFO_RES";
        jsV["payload"] = jsP;

        callScript(jsV);
    }
    else if (strcmp(szAct, "MODIFY_BASEINFO") == 0) {
        jsV["act"] = "MODIFY_BASEINFO_DONE";

        if (m_config.setBaseSetupPayload(jsPayload)) jsV["payload"] = "OK";
        else  jsV["payload"] = "ERROR";

        callScript(jsV);
    }
    else if (strcmp(szAct, "GET_SYS_CONFIG") == 0) {
        jsV["act"] = "GET_SYS_CONFIG_RES";
        if (m_config.getSetupPayload(jsP) ) jsV["payload"] = jsP;        
        else jsV["payload"] = "NONE";

        callScript(jsV);
    }
    else if (strcmp(szAct, "SET_SYS_CONFIG") == 0) {
        jsV["act"] = "SET_SYS_CONFIG_RES";
        if (m_config.setSetupPayload(jsPayload) ) jsV["payload"] = "OK";        
        else jsV["payload"] = "ERROR";

        callScript(jsV);
    }
    else if (strcmp(szAct, "GET_LOGIN_INFO") == 0) {
        jsV["act"] = "GET_LOGIN_INFO_RES";
        if (m_config.getLoginPayload(jsP) ) jsV["payload"] = jsP;        
        else jsV["payload"] = "NONE";

        callScript(jsV);
    }
    else if (strcmp(szAct, "SET_LOGIN_INFO") == 0) {
        jsV["act"] = "SET_LOGIN_INFO_RES";
        if (m_config.setLoginPayload(jsPayload) ) jsV["payload"] = "OK";        
        else jsV["payload"] = "ERROR";

        callScript(jsV);
    }
    else if (strcmp(szAct, "GET_MAINTX_LIST") == 0) {
        Json::Value jsV2;
        int iCnt = 0;

        jsV["act"] = "GET_MAINTX_LIST_RES";
        jsV2 = m_net_detector->getMTXList(iCnt);        
        if (iCnt > 0) jsV["payload"] = jsV2;
        else jsV["payload"] = "NONE";

        callScript(jsV);
    }
    else if (strcmp(szAct, "GET_LOCAL_MEDIA_LIST") == 0) {
        Json::Value jsV2;

        jsV["act"] = "GET_LOCAL_MEDIA_LIST_RES";
        if (m_local_media != NULL) m_local_media->getMediaPayload(jsV2);
        else jsV2 = "NONE";
        jsV["payload"] = jsV2;        

        callScript(jsV);
    }
    else if (strcmp(szAct, "ADD_LOCAL_MEDIA") == 0) {
        jsV["act"] = "ADD_LOCAL_MEDIA_RES";
        if (m_local_media != NULL) {
            if (m_local_media->addLocalMedia(jsPayload)) jsV["payload"] = "OK";
            else jsV["payload"] = "FAILED";
        }
        else jsV["payload"] = "ERROR: INVALID CALL";        

        callScript(jsV);
    }
    else if (strcmp(szAct, "DEL_LOCAL_MEDIA") == 0) {
        jsV["act"] = "DEL_LOCAL_MEDIA_RES";
        if (m_local_media != NULL) {
            if (m_local_media->deleteLocalMedia(jsPayload)) jsV["payload"] = "OK";
            else jsV["payload"] = "FAILED";
        }
        else jsV["payload"] = "ERROR: INVALID CALL";        

        callScript(jsV);
    }
    else if (strcmp(szAct, "GET_HARDWARE_INFO") == 0) {
        jsV["act"] = "GET_HARDWARE_INFO_RES";
        if (m_i2c != NULL) {            
            if (strcmp(jsPayload.asCString(), "adc_attr") == 0) {
                m_i2c->GetChAttributes(jsV["payload"]);
                jsV["payload"]["type"] = jsPayload;
            }
            else if (strcmp(jsPayload.asCString(), "adc_volume") == 0) {
                m_i2c->GetChVolumes(jsV["payload"]);            
                jsV["payload"]["type"] = jsPayload;
            }
            else {
                jsV["payload"] = "NONE";
                jsV["error"] = "Unknown info type payload";
            }
        }
        else {
            jsV["payload"] = "NONE";        
            jsV["error"] = "Not supported";
        }

        callScript(jsV);
    }
    else if (strcmp(szAct, "SET_HARDWARE_INFO") == 0) {
        jsV["act"] = "SET_HARDWARE_INFO_RES";
        if (m_i2c != NULL && jsPayload.isMember("type") && jsPayload.isMember("value")) {
            if (strcmp(jsPayload["type"].asCString(), "adc_volume") == 0) m_i2c->SetChVolumes(jsPayload["value"], jsV["payload"]);            
            else {
                jsV["payload"] = "NONE";
                jsV["error"] = "Unknown info type";
            }
        }
        else {
            jsV["payload"] = "NONE";        
            jsV["error"] = "Not supported";
        }

        callScript(jsV);
    }
    else if (strcmp(szAct, "GET_GPIO_VALUES") == 0) {
        jsV["act"] = "GET_GPIO_VALUES_RES";
        if (m_pGPIO != NULL && jsPayload.isMember("pins")) {
            Json::Value jsRes;
            Json::Value pins = jsPayload["pins"];
            for (int i = 0; i < pins.size(); i++) {
                Json::Value jsP; 
                int32_t iRes = 0;
                jsP["pin"] = pins[i];
                jsP["res"] = m_pGPIO->Read(pins[i].asInt(), iRes);
                jsP["value"] = iRes;
                jsRes.append(jsP);
            }
            jsV["payload"] = jsRes;
        }
        else {
            jsV["payload"] = "NONE";        
            jsV["error"] = "Not supported";
        }

        callScript(jsV);
    }
    else if (strcmp(szAct, "SET_GPIO_VALUES") == 0) {
        jsV["act"] = "SET_GPIO_VALUES_RES";
        if (m_pGPIO != NULL && jsPayload.isMember("pins")) {
            Json::Value jsRes;
            Json::Value pinv = jsPayload["pins"];
            for (int i = 0; i < pinv.size(); i++) {
                Json::Value jsP;                 
                jsP["pin"] = pinv[i]["pin"];
                jsP["res"] = m_pGPIO->Write(pinv[i]["pin"].asInt(), pinv[i]["value"].asInt());                
                jsRes.append(jsP);
            }
            jsV["payload"] = jsRes;
        }
        else {
            jsV["payload"] = "NONE";        
            jsV["error"] = "Not supported";
        }

        callScript(jsV);
    }
}

void MainLogic::runSystemCommand(Json::Value jsPayload) {
    Json::Value jsV, jsP;
    
    jsP["result"] = "ERROR";
    if (jsPayload.isMember("cmd") && jsPayload.isMember("cmd_id")) {
        char* szCmdT;
        bool bNeedReturn = false;
        jsP["result"] = "OK";
        jsP["cmd_id"] = jsPayload["cmd_id"];        
        if (jsPayload.isMember("need_result")) {
            if (jsPayload["need_result"].asBool()) bNeedReturn = true;
        }
        szCmdT = new char[DARK_MAXPATH];
        if (bNeedReturn) DARK_SNPRINTF2(szCmdT, DARK_MAXPATH, "%s > %s/shell_res.txt", jsPayload["cmd"].asCString(), m_config.getBasePath());
        else DARK_SNPRINTF2(szCmdT, DARK_MAXPATH, "%s", jsPayload["cmd"].asCString());        
        
        system((const char *)szCmdT);
        if (bNeedReturn) {
            FILE* fp = NULL;
            DARK_SNPRINTF2(szCmdT, DARK_MAXPATH, "%s/shell_res.txt", m_config.getBasePath());
            if (DARKPIF_fopen(&fp, szCmdT, "r") == true) {
                memset(szCmdT, 0, DARK_MAXPATH);
                fread(szCmdT, 1, DARK_MAXPATH, fp);
                fclose(fp);
                jsP["system_result"] = szCmdT;
            }
            else jsP["system_result"] = "NONE";
        }
        else jsP["system_result"] = "NONE";
        delete[] szCmdT;        
    }

    jsV["act"] = "SYS_RUN_COMMAND_RES";
    jsV["payload"] = jsP;    

    callScript(jsV);
}

void MainLogic::on_NetState(int32_t iState, void* pObject, void* pCBData) {
    MainLogic *pP = (MainLogic *)pCBData;
    pP->_on_NetState(iState, pObject);
}

void MainLogic::_on_NetState(int32_t iState, void* pObject) {
    Json::Value jsV, jsV2;
    int iCnt = 0;

    if (strlen(m_szLastURI) <= 0) {        
        g_print("Net state received, but page is not opened yet....\n");
        return;
    }
    if (!m_bScriptCallable) {
        g_print("on_NetState: State changed %d but script is not callable\n", iState);
        return;
    }

    jsV["act"] = "MAINTX_LIST_CHANGED";
    jsV2 = m_net_detector->getMTXList(iCnt);
    if (iCnt > 0) jsV["payload"] = jsV2;
    else jsV["payload"] = "NONE";

    callScript(jsV);
}
