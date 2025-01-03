const MainTXConnector = class main_tx_connector {
    constructor(szDanteID, baseInfo, mtxInfo, groupList, seq) {
        this.szDanteID = szDanteID;
        this.baseInfo = baseInfo;
        this.mtxInfo = mtxInfo;
        this.uniq_seq = seq;

        this.groupList = groupList;
        this.groupList.sort(this._sortByName);            
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

        this.selectedSpeakers = [];
        this.selectedGroups = [];
        this.enabledSpeakers = [];

        this.monitoringList = new Array();
        this.monitoringList.push({user: -1, bStart: false, tx_CH_idx: 0, rx_DEV_idx: 0, bForAMP: false, waitForDISP: 0});

        this.lrxErrorList = new Array();
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
        this._rest_getBroadcastState(false);
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
        return null;
    }

    getMyMediaChannel() {
        for (let i = 0; i < this.devList.length; i++) {
            if (this.devList[i].name == this.szDanteID) {
                return this.devList[i].tx_channels[0].idx;                
            }
        }
        return 0;
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

    //NODEV, OK, PART_0_0_0
    getGroupSpeakersConnState(iGrpId) {
        let totalCnt = 0;
        let okCnt = 0;
        let disCnt = 0;
        let abnCnt = 0;
        for (let edev of this.devList) {      
            for (let rxc of edev.rx_channels) {                
                if (rxc.speakers == null) continue;
                for (let espk of rxc.speakers) {
                    if (espk.groups.indexOf(iGrpId) >= 0) {
                        let tstate = edev.state;
                        totalCnt++;
                        if(tstate == 'DISCONNECTED') {
                            disCnt++;                            
                        }
                        else {
                            if (tstate == 'CONNECTED' || tstate == 'NORMAL') {
                                //스피커 연결 상태 검사
                                if (this.checkSpeakerIsInError(espk.idx)) abnCnt++;
                                else {
                                    if (rxc.state.status == 'NORMAL') okCnt++; //채널 상태도 검사
                                    else abnCnt++;
                                }
                            }
                            else {
                                abnCnt++;
                                //console.log("Dev " + this.devList[i2].name + ", state = " + tstate);
                            }
                        }
                    }
                }
            }
        }
        if (totalCnt == 0) return 'NODEV';
        if (totalCnt == okCnt) return 'OK';
        let res = `PART_${okCnt}_${disCnt}_${abnCnt}`;
        return res;
    }    

    getBCastNameByIdx(iIdx) {
        for (let i = 0; i < this.code_list.length; i++) {
            if (this.code_list[i].idx == iIdx) return this.code_list[i].name;
        }
        return 'UNKNOWN';
    }
    
    getOutputStateString(iIdx) {
        if (this.broadcast_list == null) return 'IDLE';
        let spkGrp = this.getSpeakersOfGroup(iIdx);
        let pSpk = 0;

        for (let ebd of this.broadcast_list) {
            if (ebd.state != 'PLAYING') continue;         
            for (let cbs of ebd.speakers) {
                for (let es of spkGrp) {
                    if (cbs == es.idx) pSpk++;
                }
            }
        }

        if (pSpk > 0) {
            return `${pSpk}/${spkGrp.length}`;
        }
        return 'IDLE';
    }

    getSpeakerEnabledString(iIdx) {        
        let spkGrp = this.getSpeakersOfGroup(iIdx);
        let enSpk = 0;

        for (let es of spkGrp) {
            //내가 선택한건 제외
            if (this.selectedSpeakers.indexOf(es.idx) >= 0) continue;
            if (this.enabledSpeakers.indexOf(es.idx) < 0) continue;
            enSpk++;
        }

        if (enSpk > 0) {
            return `${enSpk}/${spkGrp.length}`;
        }
        return null;
    }

    
    getSpeakersOfGroup(iGrpId) {
        let res = new Array();
        if (this.devList == null) return res;
        for (let ed of this.devList) {            
            for (let erx of ed.rx_channels) {
                if (erx.speakers == null) continue;
                for (let es of erx.speakers) {
                    if (es.groups.indexOf(iGrpId) >= 0) res.push(es);
                }
            }
        }
        return res;
    }

    onGroupSelect(iGrpId, bSelect) {
        let tGrpSpks = this.getSpeakersOfGroup(iGrpId);
        let exIdx = this.selectedGroups.indexOf(iGrpId);
        if (bSelect) {
            if (exIdx < 0) this.selectedGroups.push(iGrpId);
        }
        else {
            if (exIdx >= 0) this.selectedGroups.splice(exIdx, 1);
        }

        for(let es of tGrpSpks) {
            exIdx = this.selectedSpeakers.indexOf(es.idx);
            if (bSelect) {
                if (exIdx < 0) this.selectedSpeakers.push(es.idx);
            }
            else {
                if (exIdx >= 0) this.selectedSpeakers.splice(exIdx, 1);
            }
        }
        let tData = {speakers: this.selectedSpeakers, groups: this.selectedGroups};
        this._ws_sendUIBroadcast({event: "data_changed", data_type: "broadcast_output", data: tData});    
    }

    getDeviceByChannelIdx(iIdx, bWithChannel) {
        let i, i2;
        for (i = 0; i < this.devList.length; i++) {
            for (i2 = 0; i2 < this.devList[i].rx_channels.length; i2++) {
                if(this.devList[i].rx_channels[i2].idx == iIdx) {
                    if (bWithChannel) return {dev: this.devList[i], ch: this.devList[i].rx_channels[i2]};
                    else return this.devList[i];
                }
            }
            for (i2 = 0; i2 < this.devList[i].tx_channels.length; i2++) {
                if(this.devList[i].tx_channels[i2].idx == iIdx) {
                    if (bWithChannel) return {dev: this.devList[i], ch: this.devList[i].tx_channels[i2]};
                    else return this.devList[i];
                }
            }
        }
        return null;
    }

    getDeviceBySpeakerIdx(iIdx, bWithChannel) {
        for (let i = 0; i < this.devList.length; i++) {
            for (let i2 = 0; i2 < this.devList[i].rx_channels.length; i2++) {
                let spks = this.devList[i].rx_channels[i2].speakers;
                for (let i3 = 0; i3 < spks.length; i3++) {
                    if (parseInt(spks[i3].idx) == iIdx) {
                        if (bWithChannel) return {dev: this.devList[i], ch: this.devList[i].rx_channels[i2]};
                        else return this.devList[i];
                    }
                }
            }
        }
        return null;
    }

    checkChannelInMonitoring(chIdx) {
        for(let em of this.monitoringList) {
            if (em.user == -1) continue; //내꺼 빼고...
            if (chIdx == em.tx_CH_idx) return true;
        }
        return false;
    }    
    
    checkAMPMonitoring(chIdx) {
        let txDev = this.getDeviceByChannelIdx(chIdx, false);
        let mine = this.monitoringList[0];
        //현재 진행중인 모니터링이 이미 존재하는지 확인
        let strE = null;
        for(let em of this.monitoringList) {       
            if (em.user == -1) continue; //내 자신꺼는 무시 
            if (em.bStart == false) continue;                        
            if (em.rx_DEV_idx == mine.rx_DEV_idx) {
                strE = "모니터링 수신장치가 이미 다른 사용자에 의해 사용중입니다.";
                break;
            }
            //앰프모니터링의 경우 한번에 하나의 채널만 모니터링 할 수 있기때문에 장치가 겹치면 안된다
            //if (em.tx_CH_idx == chIdx) {
            let tDev = this.getDeviceByChannelIdx(em.tx_CH_idx, false);
            if (tDev.idx == txDev.idx) {
                strE = "모니터링 대상장치가 이미 다른 사용자에 의해 사용중입니다.";
                break;
            }
        }
        return strE;        
    }    

    getMonitoringInfo() {
        return this.monitoringList[0];
    }

    getDeviceByIdx(iIdx) {
        if(this.devList == null) return null;
        for (let dev of this.devList) {
            if (dev.idx == iIdx) return dev;
        }
        return null;
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
        //console.log("_refreshUARTList");
        for (i = 0; i < this.configList.length; i++) {            
            if (this.configList[i].code != 'UART_PORT') continue;
            //console.log("idx = " + this.configList[i].idx + ", val=" + this.configList[i].i_value);
            //if (!(this.configList[i].i_value == 1 || this.configList[i].i_value == 2)) continue;

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

            for (let ee of this.emrg_list) {
                if (targetConf.i_value != ee.proto_port) continue;
                tGroups.push(ee);
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
                    info: targetConf.str_value,
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
            //내 장치 아이디 설정 - 모니터링시 아이디를 설정한다
            for (let ed of this.devList) {
                if (ed.name == this.szDanteID) {
                    this.monitoringList[0].rx_DEV_idx = ed.idx;
                    break;
                }
            }
    

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
            this._rest_getSysState();            
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
            if (this.funcEventCallback != null) this.funcEventCallback(EVTSTR_MTX_DATA_UPDATED, this, "emrg_list");
        }.bind(this));
    }

    _rest_getBroadcastState(bNotifyChange) {        
        this.rest_call('get_broadcast_state', 'NONE', bNotifyChange, function (bRes, jsRecv, callParam) {
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
            if (callParam == true) {
                if (this.funcEventCallback != null) this.funcEventCallback(EVTSTR_MTX_DATA_UPDATED, this, "broadcast_list");                                
            }
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

    _rest_getMediaList(bNotifyChange) {        
        this.rest_call('get_media_list', 'NONE', bNotifyChange, function (bRes, jsRecv, callParam) {
            if (!bRes) {
                console.error("_rest_getBroadcastCode: Server error");
                return;
            }
            if (jsRecv.res != true) {
                console.log("Get Media list failed..." + js.error);
                return;
            }            
            this.media_list = new Array();
            for(let em of jsRecv.payload.list) {
                let spl = em.mime.split('/');
                if (spl[0] != 'audio') continue;
                this.media_list.push(em);
            }
            this.media_list.sort(this._sortByB64Title);
            this._checkAndReport();
            if (callParam == true) {
                if (this.funcEventCallback != null) this.funcEventCallback(EVTSTR_MTX_DATA_UPDATED, this, "media_list");                                
            }            
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

    _rest_getGroupList() {
        let payload = { dev_id: this.baseInfo.sys_id, dante_id: this.szDanteID };
        this.rest_call('get_group_list', payload, null, function (bRes, jsRecv, callParam) {
            if (!bRes) {
                console.error("_rest_getGroupList: Server error");
                return;
            }
            if (jsRecv.res != true) {
                console.log("Get Group list failed..." + js.error);
                return;
            }            
            this.groupList = jsRecv.payload;
            this.groupList.sort(this._sortByName);            
            if (this.funcEventCallback != null) this.funcEventCallback(EVTSTR_MTX_DATA_UPDATED, this, "group_list");
        }.bind(this));
    }

    _rest_getSysState() {        
        this.rest_call('get_sys_state', 'NONE', null, function (bRes, jsRecv, callParam) {
            if (!bRes) {
                console.error("_rest_getSysState: Server error");
                return;
            }
            if (jsRecv.res != true) {
                console.log("Get sys state failed..." + jsRecv.error);
                console.log(JSON.stringify(jsRecv));
                return;
            }
            let pld = jsRecv.payload;
            for (let ep of jsRecv.payload) {
                if (ep.name == 'uart_state') {
                    //console.log("UART State = " + ep.value);
                    this._refreshUARTState(ep.value, true);
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

    _sortByName(a, b) {
        if (a.name < b.name) return -1;
        if (a.name > b.name) return 1;
        return 0;
    }


    _refreshUARTState(szState, bUpdateScreen) {
        let i;

        //console.log("_RefreshUARTState: " + szState);
        if (this.uart_list == null) {
            //목록이 없으면 안댐..
            console.error('UART List is not created..');
            return;
        }

        if (szState == 'NONE') {
            for (i = 0; i < this.uart_list.length; i++) this.uart_list[i].state = 'DISCONNECTED';
            this._refreshUARTList();
            if (this.funcEventCallback != null) this.funcEventCallback(EVTSTR_MTX_DATA_UPDATED, this, "uart_list");
            return;
        }

        let spl = szState.split('__');
        for (i = 0; i < spl.length; i++) {
            let spl2 = spl[i].split(':');
            let idx = parseInt(spl2[0]);
            for (let i2 = 0; i2 < this.uart_list.length; i2++) {
                if (this.uart_list[i2].idx == idx) {
                    this.uart_list[i2].state = spl2[1];
                }
            }
        }
        this._refreshUARTList();
        if (bUpdateScreen && this.funcEventCallback != null) this.funcEventCallback(EVTSTR_MTX_DATA_UPDATED, this, "uart_list");
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
            let data = { act: jsV.act, error: 'NONE', resreq: false, payload: { idx: REMOTE_USER_IDX, name: this.baseInfo.sys_id, type: 'REMOTE' } };
            this._ws_sendAsJSON(data);
            console.log('Websocket initialized');
            this._iConnState = 10;
            this._reconnCount = 1;
            this._checkAndReport();
            //상태 가져오기
            this._requestRuntimeValue('spk_state');
            this._requestRuntimeValue('monitor_state');
            this._requestRuntimeValue('lrx_error_state');
        } 
        else if(jsV.act == 'update_notify') {
            let szTarget = jsV.payload.target;
            //console.log("update_notify: " + szTarget);
            if (szTarget == 'device') { //장치 상태가 변경됨.. 걍 업뎃
                this._rest_getDevList(true);                
            }
            else if (szTarget == 'broadcast') { //방송 상태 업데이트
                this._rest_getBroadcastState(true);
            }
            else if (szTarget == 'group') { //그룹 정보
                this._rest_getDevList(false); //그룹정보가 speakers에 속해있기때문에 이것도 업뎃해야 한다
                this._rest_getGroupList();
            }        
            else if (szTarget == 'user') { //사용자 정보
                console.log("User info updated");
            }
            else if (szTarget == 'emr_group') { //긴급/화재 정보
                this._rest_getEMRGroups();
                this._rest_getDevList(false); //그룹정보가 speakers에 속해있기때문에 이것도 업뎃해야 한다
            }
            else if (szTarget == 'media') { //음원
                this._rest_getMediaList(true);
            }            
            else if (szTarget == 'uart_state') { //긴급 방송 정보
                if (jsV.payload.type == 'update') {                    
                    this._refreshUARTState(jsV.payload.detail, false);
                    this._rest_getEMRGroups();
                }
                else console.error("Unknown uart_state type: " + jsV.payload.type);
            }        
            else if (szTarget == 'det_device') { //장치 검색됨 - 리모트는 무시
            }
            else if(szTarget == 'mtx_hardware') {} //MTX Hardware - 볼륨
            else if (szTarget == 'runtime_memory') { //스피커 선택 상태가 업데이트됨
                if (jsV.payload.detail == 'spk_state') { //스피커 상태가 업데이트 됨
                    this._requestRuntimeValue('spk_state');
                }
                else if(jsV.payload.detail == 'monitor_state') { //모니터링 상태가 업데이트 됨
                    this._requestRuntimeValue('monitor_state');
                }          
                else if(jsV.payload.detail == 'lrx_error_state') {
                    this._requestRuntimeValue('lrx_error_state');
                }
                else {
                    console.log('Rumtime memory updated detail = ' + jsV.payload.detail);
                    console.log(jsV.payload);  
                }          
            }
            else if (szTarget == 'config') {
                this._rest_getSysConfig();
            }
            else console.log("unknown update_notify: " + jsV.payload.target);
        }
        else if(jsV.act == 'receive_from') { //최초 접속시 이미 접속되어있던 애덜이 보내준다
        }
        else if(jsV.act == 'connection_notify') { //다른 사용자 접속 여부
        }
        else if(jsV.act == 'ui_broadcast') { //사용자가 데이터 전송
        }
        else if (jsV.act == 'get_key_res') {
            let pld = jsV.payload;
            if (pld.memKey == 'spk_state') {
                //console.log("spk state updated");
                //console.log(pld.memVal);  
                this._validateSpeakerStates(pld.memVal);
            }
            else if (pld.memKey == 'monitor_state') {
                //console.log("Monitor state updated");
                //console.log(pld.memVal);  
                this._processingMonitoringList(pld.memVal);
            }     
            else if (pld.memKey == 'lrx_error_state') {
                //console.log("Monitor state updated");
                //console.log(pld.memVal);  
                this._processingLRXErrList(pld.memVal);                
            }        
            else {
                console.log("Unknown key value");
                console.log(jsV);  
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

    _requestRuntimeValue(szValue) {
        let data = {act: 'get_key', resreq: false, payload: {memKey: szValue}};
        this._ws_sendAsJSON(data);      
    }       

    _ws_sendAsJSON(dictData) {
        if (this.websocket == null) {
            console.error('WS_ERROR: Not initialized..');
            return false;
        }
        this.websocket.send(JSON.stringify(dictData));
        return true;
    }    

    _ws_sendUIBroadcast(jsPayload) {
        let data = {act: 'ui_broadcast', resreq: false, payload: jsPayload};
        this._ws_sendAsJSON(data);
    }    

    _validateSpeakerStates(arrEnabledSpeakers) {        
        if (arrEnabledSpeakers == null) arrEnabledSpeakers = [];
    
        //일단 내꺼가 활성화 상태인데 활성 목록에 없다면 제거해야 한다
        if(this.selectedSpeakers.length > 0) {
            let bRemoved = true;
            while(bRemoved == true) { //for of 하니까 splice 했을때 제대로 연산이 안됨
                bRemoved = false;
                for(let spkIdx of this.selectedSpeakers) { 
                    if(arrEnabledSpeakers.indexOf(spkIdx) < 0) {
                        //console.log("My speaker is not in list: " + spkIdx);
                        let remIdx = this.selectedSpeakers.indexOf(spkIdx);
                        if (remIdx >= 0) this.selectedSpeakers.splice(remIdx, 1);
                        bRemoved = true;
                        break;
                    }
                }      
            }
        }

        // 리모트에서는 굳이 다른 처리는 할 필요 없는거 같다        
        this.enabledSpeakers = [...arrEnabledSpeakers];
        if (this.funcEventCallback != null) this.funcEventCallback(EVTSTR_MTX_DATA_UPDATED, this, "enabled_speakers");
    }    

    //monList = [{uidx: 2, tx: 10, rx: 2, forAmp: false}];
    _processingMonitoringList(monList) {
        //this.monitoringList.push({user: null, bStart: false, tx_CH_idx: 0, rx_DEV_idx: 0, bForAMP: false, waitForDISP: 0});
        let bChanged = false;
        //없어진 애들 삭제    
        let bExist = true;
        let i = 0;
        if (monList == null) monList = [];
        for(i = 0; i < monList.length; i++) monList[i].uidx = parseInt(monList[i].uidx);
        while(bExist) {
            bExist = false;
            i = 0;
            for(let em of this.monitoringList) {        
                let bOK = false;
                if (i == 0) { //내자신꺼는 제외
                    i++;
                    continue;
                }
                for(let el of monList) {
                    if (el.uidx == em.user && el.rx == em.rx_DEV_idx) {
                        bOK = true;
                        break;
                    }
                }
                if (!bOK) { //목록에서 없음
                    bExist = true;
                    bChanged = true;
                    this.monitoringList.splice(i, 1);
                    break;
                }
                i++;
            }
        }
        //신규 목록 추가
        let mine = this.monitoringList[0];
        for(let el of monList) {
            bExist = false;      
            for(let em of this.monitoringList) {
                if (em.user == el.uidx && em.rx_DEV_idx == el.rx) {
                    bExist = true;
                    break;
                }
            }
            if (!bExist) {
                //내가 한거네.. 리모트는 rx_DEV_idx 로 구분하낟
                if (el.rx == mine.rx_DEV_idx) bExist = true; 
            }
            if (!bExist) {
                this.monitoringList.push({user: el.uidx, bStart: true, tx_CH_idx: el.tx, rx_DEV_idx: el.rx, bForAMP: el.forAmp, waitForDISP: 0});      
                bChanged = true;
            }
        }

        console.log("Final Mon List");
        console.log(JSON.stringify(this.monitoringList));
        if (bChanged) {
            if (this.funcEventCallback != null) this.funcEventCallback(EVTSTR_MTX_DATA_UPDATED, this, "monitoring_state");
        }
        return bChanged;
    }

    _processingLRXErrList(errList) {
        if(errList == null) return;
        //초기에 이 목록을 먼저 받아오면 devList가 비어 있다. 따라서 일정 시간 이후에 다시 시도한다    
        if (this.state > 0 && this.state < 10) {
          setTimeout(this._processingLRXErrList.bind(this, errList), 100);
          return;
        }
    
        this.lrxErrorList = new Array();
        for(let ee of errList) {
          let devIdx = parseInt(ee.dev_idx);
          let speakerList = [];
          let tdev = this.getDeviceByIdx(devIdx);
          if(tdev != null) {
            for(let ec of tdev.rx_channels) {
              for(let es of ec.speakers) {
                if(es.code1 == ee.id) speakerList.push(es.idx);
              }
            }
          }
    
          let nEr = {dev_idx: devIdx, dev_name: ee.dev_name, code1: ee.id, speakerIdxs: speakerList};
          this.lrxErrorList.push(nEr);
        }           

        if (this.funcEventCallback != null) this.funcEventCallback(EVTSTR_MTX_DATA_UPDATED, this, "speaker_state");
    }

    checkSpeakerIsInError(spkIdx) {
        for(let ee of this.lrxErrorList) {
          if(ee.speakerIdxs.indexOf(spkIdx) >= 0) return true;
        }
        return false;
    }
    

}

export { MainTXConnector };
