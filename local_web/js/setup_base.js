let g_main = null;

function call_hitek_native(szMsg) {
    setTimeout(function () {
        window.webkit.messageHandlers.hitek_native.postMessage(szMsg)  
    }, 10);
    //window.webkit.messageHandlers.hitek_native.postMessage(szMsg);    
}

function callback_from_native(szMsg) {
    let jsV = null;
    try {
        jsV = JSON.parse(szMsg);
    } catch(e) {
        console.error('ERROR: Json parsing - ' + e);
        console.error('Origin text:');
        console.error(szMsg);
        return "ERROR";
    }
    if (g_main == null) {
        console.error('ERROR: Main is not ready');
        return "ERROR";
    }
    g_main.on_NativeCall(jsV);
    return "OK";
}


async function onSystemReady() {
    console.log("------- START -------!");

    let main = await import("./module/setup/ui_main.js");
    g_main = new main.UIMain(call_hitek_native);

    let data = {act: "SETUP_START", payload: "NONE"};
    call_hitek_native(JSON.stringify(data));
}

function SwitchPage(szPage) {
    if (g_main != null) g_main.switchPage(szPage);
}

function gDOM(szID) {
    return document.getElementById(szID);
}
