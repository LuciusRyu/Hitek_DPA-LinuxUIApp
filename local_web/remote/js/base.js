let g_main = null;

const DIVID_MENUBAR = "menu_bar";
const DIVID_MAINCTS = "main_cts";
const DIVID_MAINCTS_MODAL = "main_cts_modal";
const DIVID_WAITING_MODAL = "wait_loading_modal";

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

function showWaitModal(bShow) {
    let dom = gDOM(DIVID_WAITING_MODAL);
    if (dom == null) return;
    if(bShow) dom.style.display = "block";
    else dom.style.display = "none";
}

function parseLinuxVolume(szRes) {
    let spl = szRes.split(' ');
    if (spl.length < 3) {
        console.error("Linux volume parsing failed: " + jsPayload.run_result);
        return;
    }
    if (!(spl[0] == 'Simple' && spl[1] == 'mixer' && spl[2] == 'control')) {
        console.error("Linux volume parsing failed3: " + jsPayload.run_result);
        return;
    }

    let szPer = '0%';
    for(let i = 3; i < spl.length; i++) {
        spl[i] = spl[i].replace(/\n|\r/g, '');
        if (spl[i] == '[on]') {
            szPer = spl[i - 1];
            break;
        }
    }
    szPer = szPer.replace(/\[|\]|\%/g, '');
    //console.log("Linux volume = " + szPer);
    return szPer;
}
 
//PGA Gain 버전
function mainCHVolConversion(bToPercent, tVal) {
    //PGA Gain 0에서 120단계로 증폭, 0.5db단위
    //120 ~ 127은 동일
    //최상위 비트는 Mute
    
    let res = parseInt(tVal);
    //2024-11-20: 30%이상은 너무 크다, 그래서 36(120 x 0.3)까지만 조정
    const regMax = 36; //120;

    if (bToPercent === true) {
        if (res < 0) res = 0;
        if (res > 127) res = 0;
        else if (res > regMax) res = regMax;
        
        res = parseInt((100 * res) / regMax, 10);
        if (res > 100) res = 100;
    } else {
        if (res <= 0) {
            res = 128;
        }
        else {
            if (res > 100) res = 100;
            res = parseInt((regMax * res) / 100);
        }
    }

    return res;
}


/*
//ADC Control register 버전
function mainCHVolConversion(bToPercent, tVal) {
    //0에서 8단계로 감소, 1.5db단위, Mute 1 추가 9단계
    //1111(0xF)면 연결끊기
    let res = parseInt(tVal);

    if (bToPercent === true) {
        res = (res >> 3) & 0x0F;
        if (res == 0x0F) res = 0;
        if (res > 8) res = 8;
        
        res = parseInt(100 * (9 - res) / 9, 10);
        if (res > 100) res = 100;
        //console.log("Vol to Percent:" + tVal + "=" + res);
    } else {
        if (res <= 0) res = 0x0F;
        else {
            res = parseInt(9 * (100 - res) / 100, 10);        
            if (res > 8) res = 8;
        }
        res = 0x84 | (res << 3);
    }

    return res;
}
*/

