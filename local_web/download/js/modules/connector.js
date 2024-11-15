import { MainTXConnector } from "./conn_mtx.js";

const ConnectionManager = class connection_manager {
    constructor(funcCallNative) {
        this.funcCallNative = funcCallNative;        
        this.funcEventCallback = null;
        this.init_step = 0;        

        //{sys_type: 'type', status: 100, sys_id: 'id', interface: 'eth0', broadcast_port: 40311, type: 'DHCP', cur_ip: '192.168.0.2'}
        this.baseInfo = null;
        this.dante_dev_id = '';
        //[ {uuid: 'UUID', id: 'ID', ip: 'IP', vrrp: true, vrrp_id: 'VRRPID'} ]
        this.detectedList = [];
        this.checkNew = {seq: 0, list: [], step: 0, counter: 0, xhttp: null};

        this.mtxConnections = [];
    }

    Initialize(funcEventCallback) {        
        this.init_step = 0;
        this.funcEventCallback = funcEventCallback;
        this._callNative('GET_BASEINFO', 'NONE');        
    }

    on_NativeCall(jsV) {   
        if (jsV.act == 'GET_BASEINFO_RES') {
            if (jsV.payload == 'NONE') {
                alert('시스템 오류가 발생하였습니다.');
            } 
            else {                
                this.baseInfo = jsV.payload;
                this.init_step = 1;
                this._callNative('GET_SYS_CONFIG', 'NONE');
            }
        }
        else if (jsV.act == 'GET_SYS_CONFIG_RES') {
            if (jsV.payload == 'NONE') {
                alert('시스템 오류가 발생하였습니다.');
            } else {                
                this.dante_dev_id = jsV.payload.dante_dev_id;
                console.log("Received Dante DEV ID = " + this.dante_dev_id);
                this.init_step = 2;
                this._callNative('GET_MAINTX_LIST', 'NONE');                
            }
        } 
        else if (jsV.act == 'GET_MAINTX_LIST_RES' || jsV.act == 'MAINTX_LIST_CHANGED') {
            let tList = jsV.payload;
            if (tList == "NONE") tList = [];
            this._onMTXListChanged(tList);
        }
    }
    
    getAvailConnections() {
        let availList = [];
        for (let i = 0; i < this.mtxConnections.length; i++) {                
            if (this.mtxConnections[i].IsAvailable()) availList.push(this.mtxConnections[i]);
        }
        return availList;        
    }

    getMyTXDev() {
        let lst = this.getAvailConnections();
        if (lst.length < 1) return null;
        return lst[0].getMyTXDev();
    }


    _callNative(szAct, jsPayload) {
        let reqV = { act: szAct, payload: jsPayload };
        this.funcCallNative(JSON.stringify(reqV));
    }

    /* jsList = [
        {uuid: 'UUID', id: 'ID', ip: 'IP', vrrp: true, vrrp_id: 'VRRPID'}
    ]
    */
    _onMTXListChanged(jsList) {
        let listNew = [];
        let listDel = [];
        let i, j;
        let bExist = false;

        for (i = 0; i < this.detectedList.length; i++) {
            bExist = false;
            for(j = 0; j < jsList.length; j++) {
                if (this.detectedList[i].uuid == jsList[j].uuid) {
                    bExist = true;
                    break;
                }
            }
            if (!bExist) listDel.push({...this.detectedList[i]});
        }
        for (i = 0; i < jsList.length; i++) {
            bExist = false;
            for (j = 0; j < this.detectedList.length; j++) {
                if(jsList[i].uuid == this.detectedList[j].uuid) {
                    bExist = true;
                    break;
                }
            }
            if(!bExist) listNew.push({...jsList[i]});
        }

        this.detectedList = jsList;

        //끊어진 서버 목록 처리
        if (listDel.length > 0) {
            console.log("Diconnected MTX Server exist: " + listDel.length)            
            for (i = 0; i < listDel.length; i++) {
                for (j = 0; j < this.mtxConnections.length; j++) {
                    if (this.mtxConnections[j].checkID(listDel[i])) {
                        this.mtxConnections[j].Disconnect();
                        this.mtxConnections.splice(j, 1);
                        break;
                    }
                }
            }
            this._reportCurrentConnections();
        }

        //신규 서버 목록 처리
        if (listNew.length > 0) {
            console.log("New MTX Server exist: " + listNew.length)
            console.log(JSON.stringify(listNew));
            this.checkNew.seq++;
            this.checkNew.counter = -1;
            this.checkNew.step = 0;
            this.checkNew.list = listNew;                        
            this._onListCheckCmpl(this.checkNew.seq);
        }        
    }

    _onListCheckCmpl(seq) {
        if (seq != this.checkNew.seq) {
            console.log("Check new sequence canceled1..");
            return;
        }                
        this.checkNew.counter++;
        if (this.checkNew.counter >= this.checkNew.list.length) {
            console.log("Check new sequence done!");
            return;
        }
        
        let pld = {dev_id: this.baseInfo.sys_id, dante_id: this.dante_dev_id};
        let data = { act: 'get_group_list', dev_type: 'REMOTE', payload: pld };

        let xhttp = new XMLHttpRequest();
        this.checkNew.xhttp = xhttp;    
        xhttp.onreadystatechange = this._onListCheckResponse.bind(this, seq);
        xhttp.open('POST', getMTX_URI(this.checkNew.list[this.checkNew.counter].ip));
        xhttp.setRequestHeader('Content-Type', 'application/json');
        
        xhttp.send(JSON.stringify(data));
    }

    _onListCheckResponse(seq) {        
        if (seq != this.checkNew.seq) {
            console.log("Check new sequence canceled2..");
            return;
        }                
        let js = getJSON_FromXHTTP(this.checkNew.xhttp);
        if (js == null) return;
        if (js == 'ERROR') { //수신에러...아마도 서버에러??
            return;
        }
        if (js.res != true) {
            console.log("Target server is not mine...");
            this._onListCheckCmpl(seq);
            return;
        }
        //console.log(JSON.stringify(js));
        this._onNewServer(seq, js.payload);
    }

    _onNewServer(seq, groupList) {        
        if (seq != this.checkNew.seq) {
            console.log("Check new sequence canceled2..");
            return;
        }                
        console.log("New server connected");
        let newMTX = new MainTXConnector(this.dante_dev_id, this.baseInfo, this.checkNew.list[this.checkNew.counter], groupList);
        this.mtxConnections.push(newMTX);
        newMTX.Initialize(this._onMTXCallback.bind(this));

        this._onListCheckCmpl(seq);
    }

    _onMTXCallback(szEvent, srcObj, jsValue) {        
        if (szEvent == EVTSTR_MTX_CONNECTED) {            
            this._reportCurrentConnections();
        }
        else if (szEvent == EVTSTR_MTX_DATA_UPDATED) {
            if (this.funcEventCallback != null) {
                let tjs = {mtxConn: srcObj, changed: jsValue};
                this.funcEventCallback(EVTSTR_DATA_CHANGED, tjs);
            }
        }
    }

    _reportCurrentConnections() {
        if (this.funcEventCallback == null) return;
        this.funcEventCallback(EVTSTR_CONN_CHANGED, this.getAvailConnections());
    }
}

export { ConnectionManager };
