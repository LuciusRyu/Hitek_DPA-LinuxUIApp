#include <gtk/gtk.h>
#include <glib.h>
#include <webkit2/webkit2.h>
#include <sys/types.h>
#include <unistd.h>
#include "sys_config.h"
#include "net_detector.h"
#include "local_media.h"
#include "I2C_Interface.h"


#define HITEK_APP_ID    "com.pentauniverse.hitek.hitekui"


static void printHelp();


class MainLogic {
public:
    MainLogic();
    ~MainLogic();

    int StartSystem(int argc, char** argv);
    bool IsStarted() { return m_bStarted; }

protected:
    sysConfig m_config;
	pthread_attr_t m_THRATT_detach;
    bool m_bStarted;
    GtkApplication *m_pApp;
    GtkWindow *m_pWindow;
    GtkWidget* m_pWebview;
    GtkWidget *m_pVbox;
    netDetector *m_net_detector;
    localMedia *m_local_media;
    I2C_Interface *m_i2c;
    GPIO_Control *m_pGPIO;

    int m_iSysState;
    char *m_szScriptBuf;
    int m_iScriptBuf;
    DARKCRITICAL_TYPE CRITICAL_script;
    char *m_szLastURI;
    bool m_bShowInspector;
    bool m_bScriptCallable; //웹페이지에서 먼저 뭔가 호출을 해 줘야만 역 호출을 하자.

    void _THR_Loop();
    int _cmdlineCallback(GApplicationCommandLine *cmdline);
    gboolean _update_gui();
    gboolean _on_webview_load_failed(WebKitWebView  *webview, WebKitLoadEvent load_event, gchar *failing_uri, GError *error);
    void _script_callback_function(WebKitWebView *web_view, WebKitJavascriptResult *js_result);

    void prepareWindow();
    void webkitSetup(WebKitHardwareAccelerationPolicy eGPUAcc);
    void openURI(const char *szURI, bool bReload = false);
    void callScript(Json::Value &jsVal);
    void on_script_request(const char *szAct, Json::Value jsPayload);
    void runSystemCommand(Json::Value jsPayload);
    void _on_NetState(int32_t iState, void* pObject);

protected:
	static void *THR_Loop(void *pParam);    
    static int cmdlineCallback(GtkApplication *app, GApplicationCommandLine *cmdline, gpointer data);
    static gboolean on_webview_load_failed(WebKitWebView  *webview, WebKitLoadEvent load_event, gchar *failing_uri, GError *error, gpointer user_data);
    static gboolean update_gui(gpointer data);
    static WebKitUserContentManager* create_scriptCallback_mng(gpointer user_data);
    static void script_callback_function(WebKitWebView *web_view, WebKitJavascriptResult *js_result, gpointer user_data);
    static void on_javascript_finished (GObject *object, GAsyncResult *result, gpointer user_data);
    static void on_NetState(int32_t iState, void* pObject, void* pCBData);
};
