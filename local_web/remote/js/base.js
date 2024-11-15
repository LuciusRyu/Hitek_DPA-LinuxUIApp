let g_main = null;

const DIVID_MENUBAR = "menu_bar";
const DIVID_MAINCTS = "main_cts";
const DIVID_MAINCTS_MODAL = "main_cts_modal";

const MENUID_BROADCAST = "broadcast";
const MENUID_VOLUME = "volume";
const MENUID_MONITOR = "monitor";
const MENUID_SETTING = "setting";

const MAINTX_WEBSOCKET_PORT = 8090;

const EVTSTR_MTX_CONNECTED = "conn_established";
const EVTSTR_MTX_DATA_UPDATED = "data_updated";
const EVTSTR_CONN_CHANGED = "conn_state_changed";
const EVTSTR_DATA_CHANGED = "data_changed";

let g_callnative_queue = [];
let g_callnative_timer = null;
let g_callnative_counter = 0;
let g_callnative_waitfor = false;

function timerfn_call_hitek_native() {
    if (g_callnative_waitfor) return;
    if (g_callnative_queue.length < 1) {
        g_callnative_counter++;
        if (g_callnative_counter > 30) {
            //3초이상 데이터가 없으면 정지
            g_callnative_counter = 0;
            clearInterval(g_callnative_timer);
            g_callnative_timer = null;
            console.log("hitek_native: stop timer...");
        }
        return;
    }
    g_callnative_counter = 0;
    let msg = g_callnative_queue.shift();
    g_callnative_waitfor = true;
    window.webkit.messageHandlers.hitek_native.postMessage(msg);
}

function call_hitek_native(szMsg) {
    g_callnative_queue.push(szMsg);
    if (g_callnative_timer == null) {
        g_callnative_counter = 0;
        g_callnative_timer = setInterval(timerfn_call_hitek_native, 100);
    }
}
//function call_hitek_native(szMsg) {
//    setTimeout(function () {
//        window.webkit.messageHandlers.hitek_native.postMessage(szMsg)
//    }, 10);
//}

function callback_from_native(szMsg) {
    let jsV = null;
    try {
        jsV = JSON.parse(szMsg);
    } catch (e) {
        console.error("ERROR: Json parsing - " + e);
        console.error("Origin text:");
        console.error(szMsg);
        g_callnative_waitfor = false;
        return "ERROR";
    }
    if (g_main == null) {
        console.error("ERROR: Main is not ready");
        g_callnative_waitfor = false;
        return "ERROR";
    }
    g_main.on_NativeCall(jsV);
    if (jsV.act != "MAINTX_LIST_CHANGED") g_callnative_waitfor = false;
    return "OK";
}

async function onSystemReady() {
    console.log("------- START REMOTE-------!");

    let main = await import("./modules/main.js");
    g_main = new main.RemoteMain(call_hitek_native);
    g_main.Initialize();
}

function SwitchPage(szPage) {
    if (g_main != null) g_main.switchPage(szPage);
}

function gDOM(szID) {
    return document.getElementById(szID);
}

function getJSON_FromXHTTP(xhttp) {
    if (xhttp == null) return "ERROR";
    if (!(xhttp.readyState == xhttp.DONE && xhttp.status == 200)) {
        return null;
    }
    //console.log(this.xhttp.responseText);

    let jsRecv = null;
    try {
        jsRecv = JSON.parse(xhttp.responseText);
    } catch (e) {
        console.log("ERROR: Json parsing - " + e);
        console.log("Origin text:");
        console.log(xhttp.responseText);
        return "ERROR";
    }

    return jsRecv;
}

function getMTX_URI(szIP) {
    let tURI = "http://" + szIP + "/rest/hitek_dev_call.php";
    return tURI;
}

function PGArray2JSArray(szArr, isString) {
    let res = new Array();
    if (szArr == "{}") return res;
    if (szArr.length > 2 && szArr.substr(0, 1) == "{") {
        let nStr = szArr.substr(1, szArr.length - 2);
        let spl = nStr.split(",");
        for (let i = 0; i < spl.length; i++) {
            if (!isString) res.push(parseInt(spl[i]));
            else res.push(spl[i]);
        }
    }
    //else console.error('PGArray2JSArray: Invalid format: ' + szArr);
    return res;
}

function getDevModelPrefix(szFor) {
    if (szFor == "MAIN") return "DTX"; //PBR2U
    if (szFor == "PA") return "DPA"; //PUT4H
    if (szFor == "REMOTE") return "DRC";
    if (szFor == "PC") return "AVIOUSB"; //'DVS';

    return "NONE";
}
