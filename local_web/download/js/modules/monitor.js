const MonitorMain = class monitor_main {
    constructor(funcCallNative, connector) {
        this.funcCallNative = funcCallNative;        
        this.connector = connector;
        this.mtxList = null; 
        this.lastSelection = '';
        this.selectedMtx = null;
        this.selectedAmp = null;
        this.timer = null;
    }

    BuildLayout() {        
        this._refreshLayout(this.connector.getAvailConnections());            
    }

    on_NativeCall(jsV) {        
    }

    on_ConnectorEvent(szEvt, jsV) {        
        if (szEvt == EVTSTR_CONN_CHANGED) this._refreshLayout(jsV);        
    }

    onDisabled() {        
        if (this.selectedAmp != null) {            
            this._rest_startStop_mornitor(false, this.selectedAmp);
            this.selectedAmp = null;
        }        
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
                szLst += `' id='mm_mtxlist_${i}'>${mtxList[i].mtxInfo.id}</td></tr>\n`;                
            }
        }
        szLst += "</tbody></table>";

        let html = `
            <center><b>출력 모니터링</b></center><br/>
            <table width=100% height=100% border=1><tbody>
                <tr>
                    <td width=200px>
                        ${szLst}
                    </td>
                    <td><div id='mm_cts_main'></div></td>
                </tr>
            </tbody></table>
        `;
        
        gDOM(DIVID_MAINCTS).innerHTML = html;
        
        if (iLast >= 0) {            
            for (i = 0; i < mtxList.length; i++) {
                gDOM('mm_mtxlist_' + i).addEventListener('click', this._selectMTX.bind(this, i, false));
            }
            this._selectMTX(iLast, true);
        }        
    }

    _selectMTX(idx, bForceRefresh) {        
        if (idx < 0 || idx >= this.mtxList.length) return;
        if (this.lastSelection == this.mtxList[idx].mtxInfo.uuid && bForceRefresh != true) return;
        
        this.lastSelection = this.mtxList[idx].mtxInfo.uuid;
        this._refreshMTX(this.mtxList[idx]);
    }

    _refreshMTX(mtxConn) {        
        this.selectedMtx = mtxConn;

        let evtPairList = [];
        
        let szState = `
            <table border=1>
                <tbody>
                    <tr>
                        <td>
                            <div id="mm_selected_amp_state">
                                Not selected<br/>
                                Statue: <br/>
                                Limite: , HFP: <br/>
                                Power: , OUT-IMP: <br/>
                                Temp: , EM_Volume: <br/>
                            </div>
                        </td>
                    </tr>
                    <tr>
                        <td><div id="mm_selected_amp_level">출력 레벨: 0</div></td>
                    </tr>
                    <tr>
                        <td>
                            <input id="mm_selected_amp_vol" type="number" value=0/><br/>
                            <button id="mm_selected_amp_vol_change">변경</button>
                        </td>
                    </tr>
                </tbody>
            </table>
        `;
        let epf = {id: "mm_selected_amp_vol_change", fn: this._onVolumeChange.bind(this)};
        evtPairList.push(epf);

        let szAmps = "";

        for(let dev of mtxConn.devList) {
            if (!dev.name.startsWith(getDevModelPrefix('PA'))) continue;

            let devNick = dev.nick_name;
            if (devNick == "NONE") devNick = dev.name;

            szAmps += "<table border=1><tbody>";            
            szAmps += `<tr><td colspan=${dev.n_rx}>${devNick}</td></tr>`;
            szAmps += "<tr>";
            for(let rxc of dev.rx_channels) {
                let tid = "mm_amp_" + dev.idx + "_" + rxc.idx;
                epf = {id: tid, fn: this._onAmpClicked.bind(this, tid, mtxConn, dev, rxc)};
                evtPairList.push(epf);
                szAmps += `
                    <td id="${tid}" class="td_click">
                        ${rxc.nick_name}
                    </td>
                `;
            }
            szAmps += "</tr>";
            szAmps += "</tbody></table>"
        }                        
        this.selectedAmp = null;

        let html = `
            <table border=1>
                <thead>
                    <tr>
                        <th>상태</th>
                        <th>AMP 목록</th>
                    </tr>
                </thead>
                <tbody>
                    <tr>
                        <td>
                            ${szState}
                        </td>
                        <td>
                            ${szAmps}
                        </td>
                    </tr>
                </tbody>
            </table>
        `;
        gDOM('mm_cts_main').innerHTML = html;
        for (let epf of evtPairList) {
            gDOM(epf.id).addEventListener('click', epf.fn);
        }
    }    

    _controlUISelection(bEnable, szId, tdev, rxc) {        
        let dom = gDOM(szId);
        if(bEnable) {
            dom.classList.remove("td_click");
            dom.classList.add("td_sel");
        }
        else {
            dom.classList.remove("td_sel");
            dom.classList.add("td_click");
        }
        dom = gDOM("mm_selected_amp_state");
        let html;
        if (bEnable) {
            if(tdev.nick_name == "NONE") html = tdev.name + ": " + rxc.nick_name + "<br/>";
            else html = tdev.nick_name + ": " + rxc.nick_name + "<br/>";
        }
        else html = "Not selected<br/>";
        html += `
        Statue: <br/>
        Limite: , HFP: <br/>
        Power: , OUT-IMP: <br/>
        Temp: , EM_Volume: <br/>
        `;    
        dom.innerHTML = html;
    }

    _onAmpClicked(szId, mtxConn, tdev, rxc) {        
        if (this.selectedAmp != null && this.selectedAmp.szID == szId) {  //중지
            this._controlUISelection(false, this.selectedAmp.szID, tdev, rxc);
            this._rest_startStop_mornitor(false, this.selectedAmp);
            this.selectedAmp = null;
        }
        else { //변경
            if (this.selectedAmp != null) { //중지
                this._controlUISelection(false, this.selectedAmp.szID, tdev, rxc);
                this._rest_startStop_mornitor(false, this.selectedAmp);
            }
            //시작
            this._controlUISelection(true, szId, tdev, rxc);
            this.selectedAmp = {szID: szId, mtxConn: mtxConn, dev: tdev, rxc: rxc, seq: 0};
            this._rest_startStop_mornitor(true, this.selectedAmp);
        }
    }

    _onVolumeChange() {        
        let dom = gDOM("mm_selected_amp_vol");
        if (dom == null || this.selectedAmp == null) return;
        this._rest_changeVolume(this.selectedAmp, parseInt(dom.value));
    }

    _rest_changeVolume(objAmp, vol) {        
        let payload = {ch_idx: objAmp.rxc.idx, volume: vol};
        objAmp.mtxConn.rest_call('set_channel_volume', payload, null, function (bRes, jsRecv, callParam) {
            if (bRes != true) {
                alert("볼륨 제어 실패");
                return;
            }
            if (jsRecv.res != true) {
                alert("볼륨 제어 실패: " + jsRecv.error);
                return;
            }
            console.log("Volume control success");
        }.bind(this));
    }

    _rest_startStop_mornitor(bStart, objAmp) {        
        if (objAmp.mtxConn == null) return;
        let myTX = this.connector.getMyTXDev();
        if (myTX == null) return;

        if (bStart != true) {
            if (this.timer != null) clearInterval(this.timer);
            this.timer = null;
        }

        let payload = {start: bStart, ch_idx: objAmp.rxc.idx, rx_dev: myTX.idx};
        objAmp.mtxConn.rest_call('channel_monitoring', payload, bStart, function (bRes, jsRecv, callParam) {
            if (bRes) {                
                if (jsRecv.payload == null) {
                    if (callParam == true) {
                        if (jsRecv.error_code == 5) {
                            //error: Target route exist already..
                            console.log("Error code = Existing route:" + jsRecv.error);
                        }
                        else {
                            if (this.selectedAmp != null) {
                                this._controlUISelection(false, this.selectedAmp.szID, null, null);
                                this.selectedAmp = null;
                            }
                            console.log(JSON.stringify(jsRecv));
                            alert("채널 모니터링 시작 실패");
                        }
                    }
                    else {
                        alert("채널 모니터링 중지 실패");
                    }
                    return;
                }
                if (callParam == true) {
                    console.log("Start monitoring timer");
                    if (this.timer != null) clearInterval(this.timer);
                    this.timer = setInterval(this._channelRefreshLoop.bind(this), 200);
                }
            }
            else {
                alert("채널 모니터링 제어 실패");
            }
        }.bind(this));
    }

    _channelRefreshLoop() {      
        if (this.selectedAmp == null) return;  
        this._rest_refreshChannel(this.selectedAmp);
    }

    _rest_refreshChannel(objAmp) {
        let payload = {ch_idx: objAmp.rxc.idx};
        objAmp.mtxConn.rest_call('get_channel_status', payload, null, function (bRes, jsRecv, callParam) {
            if (bRes != true) {
                console.error("get_channel_status failed:");
                return;
            }
            if (jsRecv.res != true || jsRecv.payload == null) {
                console.error("get_channel_status failed2:" + jsRecv.error);
                return;
            }
            if (this.selectedAmp == null) {
                console.log("Amp is not selected");
                return;
            }
            if (jsRecv.payload.seq == this.selectedAmp.seq) {
                console.log("Data is not changed...");
                return;
            }
            this._refreshAmpState(jsRecv.payload);
        }.bind(this));
    }

    _refreshAmpState(jsRes) {        
        if (jsRes.b64_val == 'NONE') {
            console.log('Warning - No status value..');
            return;
        }
        let values = Base64.decode(jsRes.b64_val);
        let res = JSON.parse(values);
        let dom = gDOM("mm_selected_amp_state");

        let devNick = this.selectedAmp.dev.nick_name;
        if (devNick == "NONE") devNick = this.selectedAmp.dev.name;
        let html = `
        <b>${devNick}</b>: ${this.selectedAmp.rxc.nick_name}<br/>
        Statue: ${this.selectedAmp.rxc.state.status}<br/>
        Limite: ${res.limit}, HFP: ${res.hfp}<br/>
        Power: ${res.power}, OUT-IMP: ${res.imp}<br/>
        Temp: ${res.temp}, EM_Volume: ${res.em_vol}<br/>
        `;

        dom.innerHTML = html;

        dom = gDOM("mm_selected_amp_level");
        dom.innerText = "출력 레벨: " + res.level;
        //설정 볼륨값 = res.vol    
    }
}

export { MonitorMain };
