const BRDSTATE_IDLE = 0;
const BRDSTATE_CHIME_START_WAIT = 1;
const BRDSTATE_CHIME_START_PLAYING = 2;
const BRDSTATE_MEDIA_PLAYING = 3;
const BRDSTATE_CHIME_END_PLAYING = 4;
const BRDSTATE_NORMAL_PLAYING = 10;
const BRDSTATE_EXCHANGE_TX2NORMAL = 11;
const BRDSTATE_EXCHANGE_TX2MEDIA = 12;
const BRDSTATE_FINISHING = 20;

const BroadcastMain = class broadcast_main {
    constructor(funcCallNative, connector) {
        this.funcCallNative = funcCallNative;        
        this.connector = connector;       
        this.mtxList = null; 
        this.selectedMtx = null;
        this.lastSelection = '';
        this.localMedias = null;
        this.broadcast = null;
        this.chimeMedia = [];
        this.mediaPlayCount = 0;
    }

    BuildLayout() {
        this.broadcast = {groups: [], txch: 0, medias: [], state: 0, autoChime: false, media_tx: 0, broadcast_idx: 0};
        this._refreshLayout(this.connector.getAvailConnections());            
        this._getLocalMediaList();
    }

    on_NativeCall(jsV) {                
        if (jsV.act == "GET_LOCAL_MEDIA_LIST_RES") {
            if (jsV.payload != "NONE") this.localMedias = jsV.payload;
            let mtx = this._getLastMTX();
            if (mtx != null) this._refreshMTX(mtx);
        }
        else if (jsV.act == "ADD_LOCAL_MEDIA_RES") {
            console.log("Add local media done: " + jsV.payload);
            this._getLocalMediaList();
        }        
    }

    on_ConnectorEvent(szEvt, jsV) {        
        if (szEvt == EVTSTR_CONN_CHANGED) this._refreshLayout(jsV);
        else if(szEvt == EVTSTR_DATA_CHANGED) {
            if (jsV.changed == 'device_list') {
                //이럴리는 없지만 그래도 한번 검사해본다..
                let bObjInList = false;
                for(let mtx of this.mtxList) {
                    if (mtx == jsV.mtxConn) {
                        bObjInList = true;
                        break;
                    }
                }
                if (!bObjInList) {
                    console.error("변화된 객체가 현재 목록에 없음");
                    return;
                }
                if (this.lastSelection == jsV.mtxConn.mtxInfo.uuid) {
                    console.log("Need to update screen");
                    this._refreshMTX(jsV.mtxConn);
                }                
            }
            else {
                console.error("Unknown changed data: " + jsV.changed);
            }
        }
        else {
            console.error("Unknown connector event!!!: " + szEvt);
        }
    }

    _getLastMTX(idx) {        
        for (let i = 0; i < this.mtxList.length; i++) {
            if (this.lastSelection == this.mtxList[i].mtxInfo.uuid) return this.mtxList[i];
        }
        return null;
    }

    _getLocalMediaList() {
        let reqV = { act: "GET_LOCAL_MEDIA_LIST", payload: "NONE" };
        this.funcCallNative(JSON.stringify(reqV));
    }

    _refreshLayout(mtxList) {        
        let szLst = '<table border=1><tbody>';
        this.mtxList = mtxList;        
        let iLast = -1;        
        let i;
        if (mtxList.length < 1) {
            szLst += '<tr><td>서버 없음</td></tr>';
            this.lastSelection = '';
        }
        else {   
            //마지막 선택했던 서버가 아직 있나?         
            for (i = 0; i < mtxList.length; i++) {
                if (mtxList[i].mtxInfo.uuid == this.lastSelection) {
                    iLast = i;
                    break;
                }
            }            
            if(iLast < 0) iLast = 0;            
            for (i = 0; i < mtxList.length; i++) {
                szLst += "<tr><td class='";
                if (i == iLast) szLst += "td_sel";
                else szLst += "td_click";
                szLst += `' id='bm_mtxlist_${i}'>${mtxList[i].mtxInfo.id}</td></tr>\n`;                
            }
        }
        szLst += "</tbody></table>";

        let html = `
            <center><b>리모트 방송 메인 페이지</b></center><br/>
            <table width=100% height=100% border=1><tbody>
                <tr>
                    <td width=200px>
                        ${szLst}
                    </td>
                    <td><div id='bm_cts_main'></div></td>
                </tr>
            </tbody></table>
        `;
        
        gDOM(DIVID_MAINCTS).innerHTML = html;
        
        if (iLast >= 0) {
            for (i = 0; i < mtxList.length; i++) {
                gDOM('bm_mtxlist_' + i).addEventListener('click', this._selectMTX.bind(this, i));
            }
            this._selectMTX(iLast);
        }
        //console.log(gDOM(DIVID_MAINCTS).innerHTML);
    }

    _selectMTX(idx) {        
        if (idx < 0 || idx >= this.mtxList.length) return;
        if (this.lastSelection == this.mtxList[idx].mtxInfo.uuid) return;

        this.lastSelection = this.mtxList[idx].mtxInfo.uuid;
        this._refreshMTX(this.mtxList[idx]);
    }

    _onMediaDownload(mtxInfo, mediaInfo) {
        //console.log(`WTF: ${mtxInfo.ip} - ${mtxInfo.uuid} - ${mediaInfo.idx}`);
        let data = {mtx_ip: mtxInfo.ip, mtx_uuid: mtxInfo.id, remote_uri: Base64.decode(mediaInfo.b64_local), remote_idx: mediaInfo.idx, title: mediaInfo.b64_title};
        let reqV = { act: "ADD_LOCAL_MEDIA", payload: data };
        this.funcCallNative(JSON.stringify(reqV));
        console.log("Add local media called");
    }

    _findMediaByIdx(idx) {
        //로컬 음원에서 찾기                    
        for (let m of this.localMedias) {
            if (m.remote_idx == idx) return this._genMediaURI(true, m);            
        }
        //서버에서 찾기
        for (let m of mtxConn.media_list) {                        
            if (m.idx == idx) return this._genMediaURI(false, m);                
        }                        
        return null;
    }

    _onControlBroadcast(szId) {                
        if (this.broadcast.state == BRDSTATE_IDLE) {
            if(this.selectedMtx == null) return;
            let dom = gDOM("bdc_autochime");
            let bAutoChime = dom.checked;            
            this.broadcast.autoChime = bAutoChime;     
            if(bAutoChime) {                
                //시작/종료 차임 음원 찾기, idx=2, 3 으로 고정                
                if (this.chimeMedia.length < 2) {
                    this.chimeMedia = [];
                    let tM = this._findMediaByIdx(2);
                    if (tM == null) {
                        alert("시작차임 음원이 없습니다.");
                        return;
                    }
                    this.chimeMedia.push(tM);
                    tM = this._findMediaByIdx(3);
                    if (tM == null) {
                        alert("종료차임 음원이 없습니다.");
                        return;
                    }
                    this.chimeMedia.push(tM);
                }
            }
            let myTX = this.selectedMtx.getMyTXDev();
            let mediaTXC = 0;
            for (let txc of myTX.tx_channels) {
                if (txc.order == 0) {
                    mediaTXC = txc.idx;
                    break;
                }
            }
            this.broadcast.media_tx = mediaTXC;
            if (this.broadcast.medias.length > 0) { //미디어 재생이면 채널은 1번 채널로 고정                
                this.broadcast.txch = mediaTXC;
            }
            else {
                if (this.broadcast.txch == 0) {
                    alert("입력채널 또는 음원을 선택하세요");
                    return;
                }
            }
            if (this.broadcast.groups.length < 1) {
                alert("출력 그룹을 선택하세요");
                return;
            }
            dom = gDOM("bdc_startstop");
            dom.innerText = "시작 중..."
            if(bAutoChime) {                
                this.broadcast.state = BRDSTATE_CHIME_START_WAIT;
                this._rest_controlBroadcast(true, mediaTXC);
            }
            else {
                this.broadcast.state = BRDSTATE_NORMAL_PLAYING;
                this._rest_controlBroadcast(true, this.broadcast.txch);
            }
        }
        else if(this.broadcast.state == BRDSTATE_MEDIA_PLAYING) {
            this._cancelMediaPlayer();
            if (this.broadcast.autoChime) {
                let dom = gDOM("bdc_startstop");
                dom.innerText = '종료차임 재생중..';
                this.broadcast.state = BRDSTATE_CHIME_END_PLAYING;
                this._setMediaPlayer(this.chimeMedia[1]);
            }
            else this._onBroadcastDone();            
        }
        else if(this.broadcast.state == BRDSTATE_NORMAL_PLAYING) {
            if (this.broadcast.autoChime) {
                if (this.broadcast.txch != this.broadcast.media_tx) {
                    this.broadcast.state = BRDSTATE_EXCHANGE_TX2MEDIA;
                    this._rest_exchangeBroadcastTX(this.broadcast.media_tx);    
                }
                else {
                    this.broadcast.state = BRDSTATE_CHIME_END_PLAYING;
                    this._setMediaPlayer(this.chimeMedia[1]);
                }
            }
            else this._onBroadcastDone();
        }
    }

    _onGroupSelect(szDomID, idx) {        
        //this.broadcast = {groups: [], txch: 0, medias: [], state: 0};
        if (this.broadcast.state != BRDSTATE_IDLE) return;
        let dom = gDOM(szDomID);
        let exIdx = this.broadcast.groups.indexOf(idx);
        if (exIdx < 0) {
            this.broadcast.groups.push(idx);
            dom.classList.remove('td_click');
            dom.classList.add('td_sel');
        }
        else {
            this.broadcast.groups.splice(exIdx, 1);
            dom.classList.remove('td_sel');
            dom.classList.add('td_click');
        }
    }

    _onTXChannelSelect(szDomID, idx) {        
        //this.broadcast = {groups: [], txch: 0, medias: [], state: 0};
        if (this.broadcast.state != BRDSTATE_IDLE) return;
        if (this.broadcast.txch == idx) return;
        //음원이 선택되어 있으면 채널은 선택 불가
        if(this.broadcast.medias.length > 0) return;
        let dom;
        if (this.broadcast.txch > 0) {
            dom = gDOM('bdc_txch_' + this.broadcast.txch);
            dom.classList.remove('td_sel');
            dom.classList.add('td_click');
        }        
        dom = gDOM(szDomID);
        dom.classList.remove('td_click');
        dom.classList.add('td_sel');
        this.broadcast.txch = idx;
    }

    _genMediaURI(bIsLocal, mediaObj) {
        let res = null;
        if (bIsLocal) {
            res = "file://" + mediaObj.local_uri;
        }
        else {
            res = `http://${this.selectedMtx.mtxInfo.ip}${Base64.decode(mediaObj.b64_local)}`;
        }
        return res;
    }

    _onMediaSelect(szDomID, bIsLocal, mediaObj) {        
        //this.broadcast = {groups: [], txch: 0, medias: [], state: 0};        
        if (this.broadcast.state != BRDSTATE_IDLE) return;
        let uri = this._genMediaURI(bIsLocal, mediaObj);
        let exIdx = -1;
        for (let i= 0; i < this.broadcast.medias.length; i++) {
            if (this.broadcast.medias[i] == uri) {
                exIdx = i;
                break;
            }
        }
        let dom;
        //음원을 선택하면 입력채널은 선택할 수 없다
        if (this.broadcast.txch > 0) {
            dom = gDOM('bdc_txch_' + this.broadcast.txch);
            dom.classList.remove('td_sel');
            dom.classList.add('td_click');
            this.broadcast.txch = 0;
        }
        dom = gDOM(szDomID);
        if(exIdx < 0) {
            this.broadcast.medias.push(uri);
            dom.classList.remove('td_click');
            dom.classList.add('td_sel');
        }
        else {
            this.broadcast.medias.splice(exIdx, 1);
            dom.classList.remove('td_sel');
            dom.classList.add('td_click');
        }
    }

    _refreshMTX(mtxConn) {        
        let mtx = mtxConn.getMainTXDev();
        let uartState = this.generateUARTState(mtxConn);
        let brdState = this.generateBRDState(mtxConn);
        let szEmrList = '';
        let szBrdList = '';
        for (let emr of uartState.emrList) szEmrList += emr.type + ": " + emr.title + "<br/>";        
        for (let brd of brdState) szBrdList += brd.title + ": " + brd.count + "개<br/>";

        this.selectedMtx = mtxConn;

        let mtxNick = mtx.nick_name;
        if (mtxNick == 'NONE') mtxNick = mtx.name;

        let evtPairList = [];

        let szState = `
            <table border=1>
                <tbody>
                    <tr>
                        <td>
                            MainTX: ${mtxNick}<br/>
                            상태: ${mtx.state}<br/>
                        </td>
                    </tr>
                    <tr>
                        <td>
                            화재 수신기<br/>
                            ${uartState.devName}: ${uartState.devState}<br/>
                            ${szEmrList}
                        </td>
                    </tr>
                    <tr>
                        <td>
                            방송상태정보<br/>
                            ${szBrdList}
                        </td>
                    </tr>
                    <tr><td>&nbsp;</td></tr>
                    <tr>
                        <td>
                            <button id="bdc_startstop">&nbsp;&nbsp;&nbsp;방송 시작&nbsp;&nbsp;&nbsp;</button><br/>
                            <input type="checkbox" id="bdc_autochime" checked>자동 차임</input>
                        </td>
                    </tr>
                </tbody>
            </table>
        `;
        let epf = {id: "bdc_startstop", fn: this._onControlBroadcast.bind(this, "bdc_startstop")};
        evtPairList.push(epf);

        let grps = mtxConn.groupList;
        let szGroup = "<table border=1><tbody>";
        for(let i = 0; i < grps.length; i++) {
            let tid = 'bdc_group_list_' + grps[i].idx;
            szGroup += "<tr><td id='" + tid + "' class='td_click'>";
            szGroup += `[${i}]: 그룹명=${grps[i].name}, idx=${grps[i].idx}, 스피커목록=${grps[i].speakers}`;
            szGroup += "</td></tr>\n";
            epf = {id: tid, fn: this._onGroupSelect.bind(this, tid, grps[i].idx)};
            evtPairList.push(epf);
        }
        szGroup += "</tbody></table>";

        let myTX = mtxConn.getMyTXDev();
        
        let szTXC = "<table border=1><tbody>";
        for (let txc of myTX.tx_channels) {
            let tid = 'bdc_txch_' + txc.idx;
            szTXC += "<tr><td id='" + tid + "' class='td_click'>";
            szTXC += `[idx:${txc.idx}] : ${txc.nick_name}`;
            szTXC += "</td></tr>\n";
            epf = {id: tid, fn: this._onTXChannelSelect.bind(this, tid, txc.idx)};
            evtPairList.push(epf);
        }
        szTXC += "</tbody></table>";
        
        let szMedia = `
            <table border=1 cellspacing=0 cellpadding=0>
            <tbody>
                <tr><td colspan=2><div id='bdc_media_player'></div></td></tr>
                <tr><td colspan=2>서버 음원</td></tr>
        `;        
        for (let m of mtxConn.media_list) {                        
            if (m.idx > 1) {
                let tid = "bdc_media_select_" + m.idx;
                let epf = {id: "bdc_media_down_" + m.idx, fn: this._onMediaDownload.bind(this, mtxConn.mtxInfo, m)};
                let epf2 = {id: tid, fn: this._onMediaSelect.bind(this, tid, false, m)};
                szMedia += `
                <tr>
                    <td class="td_click" id="${epf2.id}">[idx:${m.idx}]: ${Base64.decode(m.b64_title)}</td>
                    <td><button id=${epf.id}>다운로드</td>
                </tr>
                `;
                evtPairList.push(epf);
                evtPairList.push(epf2);
            }
            else {
                //szMedia += `[idx:${m.idx}]: ${Base64.decode(m.b64_title)}, ${Base64.decode(m.b64_local)}<br/>\n`;
            }
        }
        if (this.localMedias != null) {
            szMedia += "<tr><td colspan=2>로컬 음원</td></tr>";
            for (let m of this.localMedias) {
                let tid = "bdc_media_select_local_" + m.mtx_uuid + "_" + m.seq;
                let epf = {id: tid, fn: this._onMediaSelect.bind(this, tid, true, m)};
                szMedia += `
                <tr>
                    <td class="td_click" id="${epf.id}" colspan=2>${Base64.decode(m.title)}</td>
                </tr>
                `;
                evtPairList.push(epf);
            }
        }
        szMedia += "</tbody></table>";

        let html = `
            <table border=1>
                <thead>
                    <tr>
                        <th>사이드바</th>
                        <th>그룹목록</th>
                        <th>입력채널</th>
                        <th>음원</th>
                    </tr>
                </thead>
                <tbody>
                    <tr>
                        <td>
                            ${szState}
                        </td>
                        <td>
                            ${szGroup}
                        </td>
                        <td>
                            ${szTXC}
                        </td>
                        <td>
                            ${szMedia}
                        </td>
                    </tr>
                </tbody>
            </table>
        `;
        gDOM('bm_cts_main').innerHTML = html;
        for (let epf of evtPairList) {
            gDOM(epf.id).addEventListener('click', epf.fn);
        }
    }

    generateUARTState(mtxConn) {
        let html = '';
        let sTemp = '';
        let i, i2;
        let tgrps = null;
        let bOccured = false;

        let res = {devName: '', devState: '', emrList: []};

        let tUart = null;
        for (i = 0; i < mtxConn.uart_list.length; i++) {
            if (mtxConn.uart_list[i].port == 1) {
                tUart = mtxConn.uart_list[i];
                break;
            }
        }
        if (tUart == null) {
            console.error('ERROR: RefreshUARTListUI - No UART Info..');
            return res;
        }

        res.devName = tUart.name;
        if (tUart.state == 'DISCONNECTED') res.devState = '연결 안됨';
        else if (tUart.state == 'SIGNAL_RELAY_DIS') res.devState = '연동 정지';
        else res.devState = '정상';

        let tName = '';
        
        for (i = 0; i < mtxConn.uart_list.length; i++) {
            tgrps = mtxConn.uart_list[i].emrGroups;
            let tgrp = null;
            for (i2 = 0; i2 < tgrps.length; i2++) {
                tgrp = tgrps[i2];
                if (!(tgrp.extra1 == 'AUTO' || tgrp.extra1 == 'MANUAL')) continue;
                if (tgrp.proto_port == 1) tName = '화재 방송';
                else tName = '긴급 방송';
                
                res.emrList.push({type: tName, title: tgrp.name});
            }
        }

        return res;
    }

    generateBRDState(mtxConn) {
        let main_count = 0;
        let remote_count = 0;
        let bgm_count = 0;
        let etc_count = 0;
        let emg_count = 0;
        let i;
        let cb;
        let res = [];
    
        if (mtxConn.broadcast_list != null) {
            for (i = 0; i < mtxConn.broadcast_list.length; i++) {
                cb = mtxConn.broadcast_list[i];
                if (cb.state == 'PLAYING') {
                    let bCate = mtxConn.getBCastCategory(cb.tx_id, cb.type_idx); //MAN, BGM, EMR, REM, PCB
                    switch (bCate) {
                    case 'MAN':
                        main_count++;
                        break;
                    case 'EMR':
                        emg_count++;
                        break;
                    case 'BGM':
                        bgm_count++;
                        break;
                    case 'REM':
                        remote_count++;
                        break;
                    case 'PCB':
                        etc_count++;
                        break;
                    }
                }
            }
        }
        res.push({title: '일반방송', count: main_count});
        res.push({title: 'BGM방송', count: bgm_count});
        res.push({title: 'PC방송', count: etc_count});
        res.push({title: '리모트방송', count: remote_count});
        res.push({title: '긴급방송', count: emg_count});
        return res;
    }    

    _rest_controlBroadcast(bStart, txch) {        
        if (this.selectedMtx == null) return;
        if (bStart) {
            let payload = {control: 'start', txc: txch, groups: this.broadcast.groups};
            this.selectedMtx.rest_call('control_broadcast', payload, null, function (bRes, jsRecv, callParam) {
                if (bRes != true || jsRecv.res != true) {
                    alert("방송제어 실패: " + jsRecv.error);
                    return;
                }
                console.log("Control res: " + JSON.stringify(jsRecv));
                this.broadcast.broadcast_idx = jsRecv.payload.broadcast_idx;                
                let dom = gDOM("bdc_startstop");
                if (this.broadcast.state == BRDSTATE_CHIME_START_WAIT) { //시작 차임 재생 대기
                    dom.innerText = '시작 차임 재생중...';
                    this.broadcast.state = BRDSTATE_CHIME_START_PLAYING;
                    this._setMediaPlayer(this.chimeMedia[0]);
                }
                else if(this.broadcast.state == BRDSTATE_NORMAL_PLAYING) { //즉시 재생
                    dom.innerText = '방송 중지';
                    if (this.broadcast.medias.length > 0) {
                        this.mediaPlayCount = 0;
                        this.broadcast.state = BRDSTATE_MEDIA_PLAYING;
                        this._setMediaPlayer(this.broadcast.medias[0]);
                    }
                }
            }.bind(this));
        }
        else {
            let payload = {control: 'stop', broadcast_idx: this.broadcast.broadcast_idx};
            this.selectedMtx.rest_call('control_broadcast', payload, null, function (bRes, jsRecv, callParam) {
                if (bRes != true || jsRecv.res != true) {
                    alert("방송제어 실패: " + jsRecv.error);
                    return;
                }
                let dom = gDOM("bdc_startstop");
                dom.innerText = '방송 시작';
                this.broadcast.state = BRDSTATE_IDLE;    
            }.bind(this));
        }
    }

    _rest_exchangeBroadcastTX(txch) {
        let payload = {control: 'exchange_tx', txc: txch, broadcast_idx: this.broadcast.broadcast_idx};
        this.selectedMtx.rest_call('control_broadcast', payload, null, function (bRes, jsRecv, callParam) {
            if (bRes != true || jsRecv.res != true) {
                alert("방송제어 실패: " + jsRecv.error);
                return;
            }
            if (this.broadcast.state == BRDSTATE_EXCHANGE_TX2NORMAL) {
                this.broadcast.state = BRDSTATE_NORMAL_PLAYING;
            }                        
            else if (this.broadcast.state == BRDSTATE_EXCHANGE_TX2MEDIA) {
                this.broadcast.state = BRDSTATE_CHIME_END_PLAYING;
                this._setMediaPlayer(this.chimeMedia[1]);
            }
        }.bind(this));
    }


    _setMediaPlayer(uri) {
        let dom = gDOM("bdc_media_player");
        console.log("Set media: " + uri);
        if (uri == null) {
            dom.innerHTML = "";            
        }
        else {
            dom.innerHTML = `<audio controls autoplay style="width: 80%; height: 20px; display: block;" id="bdc_media_play_actl"><source src="${uri}"></audio>`;                                
            dom = gDOM("bdc_media_play_actl");
            dom.addEventListener('ended', this._onMediaPlayed.bind(this));
        }
    }

    _cancelMediaPlayer() {
        let dom = gDOM("bdc_media_play_actl");
        if (dom != null) dom.pause();
        this._setMediaPlayer(null);
    }

    _onMediaPlayed() {        
        console.log("On media played");
        let dom = gDOM("bdc_startstop");
        if (this.broadcast.state == BRDSTATE_CHIME_START_PLAYING) { //시작차임 재생 완료            
            dom.innerText = '방송 중지';
            if (this.broadcast.medias.length > 0) {
                this.mediaPlayCount = 0;
                this.broadcast.state = BRDSTATE_MEDIA_PLAYING; //미디어 재생 대기
                this._setMediaPlayer(this.broadcast.medias[0]);
            }
            else {                 
                this._setMediaPlayer(null);
                if (this.broadcast.txch != this.broadcast.media_tx) {
                    //채널 변경                
                    this.broadcast.state = BRDSTATE_EXCHANGE_TX2NORMAL;
                    this._rest_exchangeBroadcastTX(this.broadcast.txch);
                }                
                else {
                    this.broadcast.state = BRDSTATE_NORMAL_PLAYING;
                }                
            }
        }
        else if (this.broadcast.state == BRDSTATE_MEDIA_PLAYING) { //미디어 재생 완료            
            this.mediaPlayCount++;
            if (this.mediaPlayCount >= this.broadcast.medias.length) {
                if (this.broadcast.autoChime) {
                    dom.innerText = '종료차임 재생중...';
                    this.broadcast.state = BRDSTATE_CHIME_END_PLAYING; //종료차임 재생 대기
                    this._setMediaPlayer(this.chimeMedia[1]);
                }
                else this._onBroadcastDone();
            }
            else {
                this._setMediaPlayer(this.broadcast.medias[this.mediaPlayCount]);
            }
        }
        else if (this.broadcast.state == BRDSTATE_CHIME_END_PLAYING) { //종료차임 재생 완료
            this._onBroadcastDone();
        }
    }

    _onBroadcastDone() {
        let dom = gDOM("bdc_startstop");
        dom.innerText = '종료중...';
        this.broadcast.state = BRDSTATE_FINISHING;
        this._setMediaPlayer(null);
        this._rest_controlBroadcast(false, this.broadcast.txch);
    }
}

export { BroadcastMain };
