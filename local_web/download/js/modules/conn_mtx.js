const MainTXConnector = class main_tx_connector {
    constructor(szDanteID, baseInfo, mtxInfo, groupList) {
        this.szDanteID = szDanteID;
        this.baseInfo = baseInfo;
        this.mtxInfo = mtxInfo;

        this.groupList = groupList;
        this.devList = null;
        this.configList = null;
        this.emrg_list = null;
        this.uart_list = null;
        this.broadcast_list = null;
        this.code_list = null;     
        //{'idx', 'b64_title', 'mime', 'b64_remote', 'b64_local', 'size', 'uidx', 'usage', 'status', 'dt_reg', 'dt_mod', 'playtime'}
        this.media_list = null;   

        this.state = 0;
        this.websocket = null;

        this._iConnState = 0;
        this._reconnCount = 0;
        this._reconnTimeout = null;
        this._srvURI = '';    

        this.funcEventCallback = null;
        this.restQueue = [];
        this.restQueue_inAction = false;
        this.restQueue_timer = null;        
    }

    checkID(mtxInfo) {
        if (mtxInfo.uuid == this.mtxInfo.uuid) return true;
        return false;
    }

    Initialize(eventCallback) {        
        this.state = 1;
        this.funcEventCallback = eventCallback;
        this._rest_getDevList(false);
        this._rest_getSysConfig();
        this._rest_getEMRGroups();
        this._rest_getBroadcastState();
        this._rest_getBroadcastCode();
        this._rest_getMediaList();
        this.Websocket_connect();
    }

    Disconnect() {     
        this.state = 0;

        if (this._reconnTimeout != null) {
            clearTimeout(this._reconnTimeout);
            this._reconnTimeout = null;
        }        
        this._reconnCount = 0;
        if (this.websocket != null) {
            this.websocket.close();          
            this.websocket = null;
        }   
    }

    Websocket_connect() {    
        this._srvURI = 'ws://' + this.mtxInfo.ip + ':' + MAINTX_WEBSOCKET_PORT;
        this._Websocket_connect();
    }

    IsAvailable() {
        if (this.state < 10) return false;
        //장치목록에서 내 단테 아이디가 있어야만 올바른 MTX다
        let bExist = false;
        for (let i = 0; i < this.devList.length; i++) {
            if (this.devList[i].name == this.szDanteID) {
                bExist = true;                
                break;
            }
        }
        if (!bExist) {
            console.log("This MainTX has no Valid Remote device " + this.szDanteID);
            return false;
        }
        return true;
    }

    getMainTX_IDX() {
        if(this.configList == null) return 0;
        for (let i = 0; i < this.configList.length; i++) {
            if (this.configList[i].code == 'TX_DEV_NAME') return this.configList[i].i_value;
        }
        return 0;
    }
    
    getMainTXDev() {
        let mtx_idx = this.getMainTX_IDX();
        if (mtx_idx <= 0) return null;
        let tdev = null;
        for (let i = 0; i < this.devList.length; i++) {
            if (parseInt(this.devList[i].idx) == mtx_idx) {
                tdev = this.devList[i];
                break;
            }
        }
        return tdev;        
    }

    getMyTXDev() {
        for (let i = 0; i < this.devList.length; i++) {
            if (this.devList[i].name == this.szDanteID) return this.devList[i];
        }
    }

    getChannelById(iIdx) {
        let i, i2;
        for (i2 = 0; i2 < this.devList.length; i2++) {
            let objDev = this.devList[i2];
            for (i = 0; i < objDev.tx_channels.length; i++) {
                if (parseInt(objDev.tx_channels[i].idx) == iIdx) return objDev.tx_channels[i];
            }
            for (i = 0; i < objDev.rx_channels.length; i++) {
                if (parseInt(objDev.rx_channels[i].idx) == iIdx) return objDev.rx_channels[i];
            }
        }
        return null;
    }
    

    getTXCHFromConfigByBCode(iBCode) {
        for (let i = 0; i < this.configList.length; i++) {
            if (this.configList[i].code != 'INPUT_CH') continue;
            if (this.configList[i].iprop == iBCode) {
                return this.configList[i].i_value;
            }
        }
        return -1;
    }
    
    getBCastTypeIndex(szType) {
        let tcode = szType;
        if (szType == 'NORMAL') tcode = 'NOR-ETC';
        else if (szType == 'MEDIA') tcode = 'NOR-MED';
        else if (szType == 'REMOTE') tcode = 'REM-NOR';
        else if (szType == 'SCHEDULE') tcode = 'NOR-SCH';
        else if (szType == 'EMR_MAN') tcode = 'EMR-MAN';
        else if (szType == 'BGM') tcode = 'NOR-BGM';
        else if (szType == 'PC') tcode = 'NOR-PCB';
    
        for (let i = 0; i < this.code_list.length; i++) {
            if (this.code_list[i].code == tcode) return parseInt(this.code_list[i].idx);
        }
        return 0;
    }

    getBCastCodeByIdx(iIdx) {
        for (let i = 0; i < this.code_list.length; i++) {
            if (this.code_list[i].idx == iIdx) return this.code_list[i].code;
        }
        return 'UNKNOWN';
    }
        
    getBCastCategory(txid, type_idx) { //MAN, BGM, EMR, REM, PCB
        let i, i2;    
        let bMedia = false;
        let mediaTxid = this.getTXCHFromConfigByBCode(this.getBCastTypeIndex('MEDIA'));
        let typeCode = this.getBCastCodeByIdx(type_idx);
        //console.log(`txid=${txid}, mtxc=${mediaTxid}, type_idx=${type_idx}, bgmidx=${this.getBCastTypeIndex('BGM')}`);
        if (txid <= 0 || txid == mediaTxid) bMedia = true; //음원 채널이거나 0 일경우
    
        if (type_idx == this.getBCastTypeIndex('BGM')) return 'BGM';      
        if (type_idx == this.getBCastTypeIndex('PC')) return 'PCB';
        if (typeCode.startsWith('EM')) return 'EMR';
    
        if (bMedia == false) {
            let tdev = null;
            let mtx_idx = this.getMainTX_IDX();
            for (i = 0; i < this.devList.length; i++) {
                for (i2 = 0; i2 < this.devList[i].tx_channels.length; i2++) {
                    if(this.devList[i].tx_channels[i2].idx == txid) {
                        tdev = this.devList[i];
                        break;
                    }
                }
            }
            if (tdev == null) {
                console.log("ERROR: TXID=" + txid + " is not exist in memory..");
                return 'REM';
            }
            if (tdev.idx == mtx_idx) {
                if (typeCode.startsWith('EM')) return 'EMR';
                return 'MAN';
            }
            else { //메인이 아니면..
                if (tdev.name.startsWith(getDevModelPrefix('PC'))) return 'PCB';
                return 'REM';
            }
        }
        else { 
            return 'MAN';
        }
    }    

    _checkAndReport() {        
        if (this.state == 1) {
            let bOK = true;
            if (this.devList == null) bOK = false;
            if (this.configList == null) bOK = false;
            if (this.emrg_list == null) bOK = false;
            if (this.broadcast_list == null) bOK = false;
            if (this.code_list == null) bOK = false;
            if (this.media_list == null) bOK = false;
            if (this._iConnState < 10) bOK = false;            

            if (bOK) {
                this.state = 10;
                if (this.funcEventCallback != null) this.funcEventCallback(EVTSTR_MTX_CONNECTED, this, null);                
            }
        }
    }

    _refreshUARTList() {
        if (this.configList == null) return;
        if (this.emrg_list == null) return;

        let i, i2;
        let targetConf = null;
        let preList = this.uart_list;
        this.uart_list = new Array();
        for (i = 0; i < this.configList.length; i++) {
            if (this.configList[i].code != 'UART_PORT') continue;
            if (!(this.configList[i].i_value == 1 || this.configList[i].i_value == 2)) continue;

            targetConf = this.configList[i];

            let objExist = null;
            if (this.uart_list.length > 0) {
                for (i2 = 0; i2 < this.uart_list.length; i2++) {
                    if (this.uart_list[i2].idx == targetConf.idx) {
                        objExist = this.uart_list[i2];
                        break;
                    }
                }
            }

            let tGroups = new Array();
            if (objExist != null) tGroups = objExist.emrGroups;

            for (i2 = 0; i2 < this.emrg_list.length; i2++) {
                if (targetConf.i_value != this.emrg_list[i2].proto_type) continue; //proto_type 1 = UART, 2 = GPIO
                tGroups.push(this.emrg_list[i2]);
            }

            if (objExist == null) {      
                let tState = 'DISCONNECTED';
                if (preList != null) {
                    for (i2 = 0; i2 < preList.length; i2++) {
                        if (preList[i2].idx == targetConf.idx) {
                            tState = preList[i2].state;
                            break;
                        }
                    }
                }
                this.uart_list.push({
                    idx: targetConf.idx,
                    state: tState,
                    relay: true,
                    name: targetConf.name,
                    emrGroups: tGroups,
                    port: targetConf.i_value,
                });
            }
        }
    }  

    _rest_getDevList(bNotifyChange) {        
        this.rest_call('get_device_list', 'NONE', bNotifyChange, function (bRes, jsRecv, callParam) {
            if (!bRes) {
                console.error("_rest_getDevList: Server error");
                return;
            }
            if (jsRecv.res != true) {
                console.log("Get Amp list failed..." + js.error);
                return;
            }
            let i1, i2;
            let list = jsRecv.payload;
            for (i1 = 0; i1 < list.length; i1++) {
                for (i2 = 0; i2 < list[i1].rx_channels.length; i2++) {
                    let rxc = list[i1].rx_channels[i2];
                    rxc.state = {status: 'NORMAL', e_code: 0 }; //상태 코드 추가
                    if (rxc.speakers == null) continue;
                    for (let i3 = 0; i3 < rxc.speakers.length; i3++) {
                        rxc.speakers[i3].groups = PGArray2JSArray(rxc.speakers[i3].groups, false);
                        rxc.speakers[i3].arrProps = PGArray2JSArray(rxc.speakers[i3].props, false); //귀찮으니 먼저 문자열을 배열로 변화해 놓는다, arrProps 추가
                    }
                }
                for (i2 = 0; i2 < list[i1].tx_channels.length; i2++) {
                    list[i1].tx_channels[i2].state = {status: 'NORMAL', e_code: 0 }; //상태 코드 추가        
                }
            }                        
            this.devList = list;
            this.devList.sort(this._sortByNickname);            
            this._checkAndReport();
            if (callParam == true) {
                if (this.funcEventCallback != null) this.funcEventCallback(EVTSTR_MTX_DATA_UPDATED, this, "device_list");                                
            }
            //채널목록도 업데이트 하자
            this._rest_updateChannelState();
        }.bind(this));
    }

    _rest_getSysConfig() {        
        this.rest_call('get_system_config', 'NONE', null, function (bRes, jsRecv, callParam) {
            if (!bRes) {
                console.error("_rest_getSysConfig: Server error");
                return;
            }
            if (jsRecv.res != true) {
                console.log("Get config list failed..." + js.error);
                return;
            }
            this.configList = jsRecv.payload.list;
            for (let i = 0; i < this.configList.length; i++) {
                this.configList[i].arr_value = PGArray2JSArray(this.configList[i].arr_value);
            }
            this._refreshUARTList();
            this._checkAndReport();
        }.bind(this));
    }

    _rest_getEMRGroups() {        
        this.rest_call('get_emrgroup_list', 'NONE', null, function (bRes, jsRecv, callParam) {
            if (!bRes) {
                console.error("_rest_getEMRGroups: Server error");
                return;
            }
            if (jsRecv.res != true) {
                console.log("Get EMR list failed..." + js.error);
                return;
            }            
            this.emrg_list = jsRecv.payload.list;
            for (let i = 0; i < this.emrg_list.length; i++) {
                this.emrg_list[i].arr_media = PGArray2JSArray(this.emrg_list[i].arr_media);
            }
            this._refreshUARTList();
            this._checkAndReport();
        }.bind(this));
    }

    _rest_getBroadcastState() {        
        this.rest_call('get_broadcast_state', 'NONE', null, function (bRes, jsRecv, callParam) {
            if (!bRes) {
                console.error("_rest_getBroadcastState: Server error");
                return;
            }
            if (jsRecv.res != true) {
                console.log("Get Broadcast list failed..." + js.error);
                return;
            }            
            this.broadcast_list = jsRecv.payload;            
            for (let i = 0; i < this.broadcast_list.length; i++) {
                this.broadcast_list[i].speakers = PGArray2JSArray(this.broadcast_list[i].speakers);
                this.broadcast_list[i].groups = PGArray2JSArray(this.broadcast_list[i].groups);
                this.broadcast_list[i].arr_media = PGArray2JSArray(this.broadcast_list[i].arr_media);
            }
            this._checkAndReport();
        }.bind(this));
    }

    _rest_getBroadcastCode() {        
        this.rest_call('get_broadcast_code', 'NONE', null, function (bRes, jsRecv, callParam) {
            if (!bRes) {
                console.error("_rest_getBroadcastCode: Server error");
                return;
            }
            if (jsRecv.res != true) {
                console.log("Get Broadcast code failed..." + js.error);
                return;
            }            
            this.code_list = jsRecv.payload.list;
            this._checkAndReport();
        }.bind(this));
    }

    _rest_getMediaList() {        
        this.rest_call('get_media_list', 'NONE', null, function (bRes, jsRecv, callParam) {
            if (!bRes) {
                console.error("_rest_getBroadcastCode: Server error");
                return;
            }
            if (jsRecv.res != true) {
                console.log("Get Media list failed..." + js.error);
                return;
            }            
            this.media_list = jsRecv.payload.list;
            this.media_list.sort(this._sortByB64Title);
            this._checkAndReport();
        }.bind(this));
    }

    _rest_updateChannelState() {        
        let payload = { ch_idx: 0 };
        this.rest_call('get_channel_status', payload, null, function (bRes, jsRecv, callParam) {
            if (!bRes) {
                console.error("_rest_updateChannelState: Server error");
                return;
            }
            if (jsRecv.res != true) {
                console.log("Get Update channgel failed..." + js.error);
                return;
            }          
            if (jsRecv.payload.hasOwnProperty('count') && jsRecv.payload.hasOwnProperty('list')) { //전체 채널 목록을 가져올때만..
                if (jsRecv.payload.count > 0) {
                    //chStateList = {ch_idx, seq, b64_val, b64_state, dt_mod};
                    let chStateList = jsRecv.payload.list;
                    for (let chs of chStateList) {
                        if (chs.b64_state == 'NONE') continue;
                        let tch = this.getChannelById(chs.ch_idx);
                        if (tch != null) {        
                            tch.state = JSON.parse(Base64.decode(chs.b64_state)); //b64_state = {status, e_code}
                        }
                    }                      
                }
            }      
        }.bind(this));
    }


    ///////////////////////////////////////////////////////////////////////////
    //Rest core
    rest_call(szAct, jsPayload, callParam, funcCallback) {
        let jsData = {act: szAct, dev_type: 'REMOTE', payload: jsPayload};
        let restObj = {xhttp: null, data: jsData, callbackParam: callParam, funcResponse: funcCallback};
        this.restQueue.push(restObj);
        if (this.restQueue_timer == null) {
            this._rest_core_loop();
            this.restQueue_timer = setInterval(this._rest_core_loop.bind(this), 100);
        }
    }

    _rest_core_loop() {                
        if (this.restQueue.length < 1) {
            clearInterval(this.restQueue_timer);
            this.restQueue_timer = null;
            return;
        }
        if (this.restQueue_inAction) return;
        let restObj = this.restQueue.shift();
        this._rest_core_call(restObj);
    }

    _rest_core_call(restObj) {        
        this.restQueue_inAction = true;
        restObj.xhttp = new XMLHttpRequest();
        restObj.xhttp.onreadystatechange = this._rest_core_response.bind(this, restObj);
        restObj.xhttp.open('POST', getMTX_URI(this.mtxInfo.ip));
        restObj.xhttp.setRequestHeader('Content-Type', 'application/json');        
        restObj.xhttp.send(JSON.stringify(restObj.data));
    }

    _rest_core_response(restObj) {
        let js = getJSON_FromXHTTP(restObj.xhttp);
        if (js == null) return;        
        if (js == 'ERROR') { //수신에러...아마도 서버에러??            
            restObj.funcResponse(false, null, restObj.callbackParam);
        }
        else restObj.funcResponse(true, js, restObj.callbackParam);
        restObj.xhttp = null;
        this.restQueue_inAction = false;
    }
    //Rest core
    ///////////////////////////////////////////////////////////////////////////

    _sortByNickname(a, b) {
        let at = (a.nick_name == 'NONE') ? a.name : a.nick_name;
        let bt = (b.nick_name == 'NONE') ? b.name : b.nick_name;
        if (at < bt) return -1;
        if (at > bt) return 1;
        return 0;
    }

    _sortByB64Title(a, b) {
        let at = Base64.decode(a.b64_title);
        let bt = Base64.decode(b.b64_title);
        if (at < bt) return -1;
        if (at > bt) return 1;
        return 0;
    }
    
    
    _Websocket_connect() {
        this._reconnTimeout = null;

        console.log('Websocket: try to connect ' + this._srvURI + ", reconn:" + this._reconnCount);
        try {
            this.websocket = new WebSocket(this._srvURI);
            this.websocket.onopen = this._Websocket_onOpend.bind(this);
            this.websocket.onclose = this._Websocket_onClosed.bind(this);
            this.websocket.onmessage = this._WebSocket_onMessage.bind(this);
            this._iConnState = 1;
        } catch (e) {
            console.log('ERROR: Websocket creation failed - ' + e);
            if (this._reconnCount > 0) this._Websocket_reconnect();
            return;
        }
    
        return true;            
    }

    _Websocket_onOpend(event) {        
        this._iConnState = 2;
        console.log('Websocket connection complete');
    }
    
    _Websocket_onClosed(event) {
        this._iConnState = 1;
        console.log('Websocket connection closed');    
        this._WebSocket_reconnect();
    }

    _WebSocket_onMessage(event) {
        //console.log('Websocket Message Recv: ' + event.data);
        let jsV = null;
        try {
            jsV = JSON.parse(event.data);
        } 
        catch (e) {
            console.log('WSI_ERROR: Json parsing - ' + e);
            return;
        }

        if (jsV.act == 'who_are_you') {
            let data = { act: jsV.act, error: 'NONE', resreq: false, payload: { idx: 9999999, name: this.baseInfo.sys_id, type: 'REMOTE' } };
            this.websocket.send(JSON.stringify(data));
            console.log('Websocket initialized');
            this._iConnState = 10;
            this._reconnCount = 1;
            this._checkAndReport();
        } 
        else if(jsV.act == 'update_notify') {
            if (jsV.payload.target == 'device') { //장치 상태가 변경됨.. 걍 업뎃
                this._rest_getDevList(true);                
            }
        }
        else {
            console.log("Websocket unknown message..." + jsV.act);            
        }
    }    

    _WebSocket_reconnect() {
        if (this._reconnCount <= 0) return;
        if (this._reconnTimeout != null) return;
        this._reconnCount++;
        this._reconnTimeout = setTimeout(this._Websocket_connect.bind(this), 1000);
    }
    
}

export { MainTXConnector };
