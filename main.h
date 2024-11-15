#include <gtk/gtk.h>
#include <glib.h>
#include <webkit2/webkit2.h>
#include <sys/types.h>
#include <unistd.h>


extern GtkWidget* g_webview;
extern GtkWindow *g_window;

static gboolean on_webview_load_failed(WebKitWebView  *webview, WebKitLoadEvent load_event, gchar *failing_uri, GError *error, gpointer user_data);
static void Setup_ConsoleLog(WebKitWebView *pWebview);
GtkWindow *prepareWindow(GtkApplication *app, GtkWidget **OUT_pWebview);
void openPage(GtkWidget* webview, gchar *cURL, bool bConsoleLog);
