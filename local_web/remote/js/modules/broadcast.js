import { PentaVolumeSlide } from "./penta_volume_slider.js";

const BRDSTATE_IDLE = 0;
const BRDSTATE_CHIME_START_WAIT = 1;
const BRDSTATE_CHIME_START_PLAYING = 2;
const BRDSTATE_CHIME_START_ONLY = 3;
const BRDSTATE_MEDIA_PLAYING = 4;
const BRDSTATE_CHIME_END_PLAYING = 5;
const BRDSTATE_NORMAL_PLAYING = 10;
const BRDSTATE_EXCHANGE_TX2NORMAL = 11;
const BRDSTATE_EXCHANGE_TX2MEDIA = 12;
const BRDSTATE_FINISHING = 20;

const COLOR_MULTITAB_NOR = "#232326";
const COLOR_MULTITAB_SEL = "#343437";

const GPIOSIGNAL_EMR = 4;
const GPIOSIGNAL_ONAIR = 6;
const GPIOSIGNAL_MIC = 8;
const GPIOSIGNAL_MEDIA = 10;

const BroadcastMain = class broadcast_main {
    constructor(funcCallNative, connector) {
        this.funcCallNative = funcCallNative;
        this.connector = connector;
        this.mtxList = null;
        this.selectedMtx = null;
        this.lastSelection = {uuid: "", dom_id: ""};
        this.localMedias = null;
        this.broadcast = null;
        this.chimeMedia = [];
        this.mediaPlayCount = 0;
        this.slideInterval = null;
        this.mSelectedSound = [];
        this.totalSeconds = 0;
        this.intervalId = null;
        this.outSelection = [];
        this.controlESPK_List = [];
        this.volumeSliders = [];
        this.volumeSlideType = 0;
        this.grid_scroll_pos = 0;
        this.input_scroll_pos = 0;
        this.preReadyState = false;     
        this.localMediaDOM_IDs = [];   
    }

    BuildLayout() {
        this.outSelection = [];
        this.broadcast = { groups: [], txch: 0, medias: [], state: 0, autoChime: false, media_tx: 0, broadcast_idx: 0, gpio_signals: []};
        this._refreshLayout(this.connector.getAvailConnections());
        this._getLocalMediaList();
    }

    on_NativeCall(jsV) {
        if (jsV.act == "GET_LOCAL_MEDIA_LIST_RES") {
            if (jsV.payload != "NONE") this.localMedias = jsV.payload;
            let mtx = this._getLastMTX();
            if (mtx != null) this._refreshMTX(mtx);
        } else if (jsV.act == "ADD_LOCAL_MEDIA_RES") {
            console.log("Add local media done: " + jsV.payload);
            this._getLocalMediaList();
        } else if (jsV.act == "DEL_LOCAL_MEDIA_RES") {
            console.log("Del local media done: " + jsV.payload);
            this._getLocalMediaList();
        }
        else if (jsV.act == "GET_HARDWARE_INFO_RES") {
            if (jsV.payload == "NONE") {
                console.error("GET_HARDWARE_INFO failed: " + jsV.error);
                return;
            }
            if (jsV.payload.type == "adc_volume") {
                //volumes = [{channel: 0, volume: 0}, ... ]                
                let volumes = jsV.payload.volumes;
                let cVolumes = [];
                if (volumes == null) {
                    console.error("Get Channel volume failed...");
                    return;
                }

                console.log("Received volume:\n" + JSON.stringify(volumes));
                for (let v of volumes) {
                    let cv = mainCHVolConversion(true, v.volume);
                    cVolumes.push({ channel: v.channel, volume: cv / 100.0 });
                }

                this._setChannelVolume(cVolumes);
            } 
            else {
                console.error("Unkown type...");
                console.log(JSON.stringify(jsV));
            }
        } 
        else if (jsV.act == "SET_HARDWARE_INFO_RES") {
        }
        else if (jsV.act == "SYS_RUN_COMMAND_RES") {
            //console.log("Sys run command");
            //console.log(JSON.stringify(jsV));
            if (jsV.payload.cmd_id == 100 && jsV.payload.system_result.length > 0) {
                this._parseLinuxVolume(Base64.decode(jsV.payload.system_result));
            }
            else if (jsV.payload.cmd_id == 101) {
                console.log("Linux volume set - done");
            }
        }        
    }

    on_ConnectorEvent(szEvt, jsV) {
        if (szEvt == EVTSTR_CONN_CHANGED) this._refreshLayout(jsV);
        else if (szEvt == EVTSTR_DATA_CHANGED) {
            //이럴리는 없지만 그래도 한번 검사해본다..
            let bObjInList = false;
            for (let mtx of this.mtxList) {
                if (mtx == jsV.mtxConn) {
                    bObjInList = true;
                    break;
                }
            }
            if (!bObjInList) {
                //이거 자주 발생
                //console.error("변화된 객체가 현재 목록에 없음");
                return;
            }

            switch(jsV.changed)
            {
            case 'device_list':
            case 'group_list':
            case 'media_list':
            case 'uart_list':
            case 'enabled_speakers':
            case 'emrg_list':
            case 'monitoring_state':
            case 'speaker_state':
                if (this.lastSelection.uuid == jsV.mtxConn.mtxInfo.uuid) {
                    console.log("Need to update screen");
                    this._refreshMTX(jsV.mtxConn);
                }
                break;
            case 'broadcast_list':
                if (this.lastSelection.uuid == jsV.mtxConn.mtxInfo.uuid) {
                    this._refreshMTX(jsV.mtxConn);                    
                    this._checkBroadcastList();
                }
                break;
            default:
                console.error("Unknown changed data: " + jsV.changed);
                break;
            }
        } else {
            console.error("Unknown connector event!!!: " + szEvt);
        }
    }

    _getLastMTX(idx) {
        for (let i = 0; i < this.mtxList.length; i++) {
            if (this.lastSelection.uuid == this.mtxList[i].mtxInfo.uuid) return this.mtxList[i];
        }
        return null;
    }

    _getLocalMediaList() {
        let reqV = { act: "GET_LOCAL_MEDIA_LIST", payload: "NONE" };
        this.funcCallNative(JSON.stringify(reqV));
    }

    _refreshLayout(mtxList) {
        let html = "";
        this.mtxList = mtxList;
        let iLast = -1;
        let i;
        html += `
            <div class="flex items-center h-[60px] bg-[#232326] rounded-t-lg">
                <div class="p-[8px] w-[148px] h-full">
                    <div class="items-center justify-center w-full h-full bg-[#343437] flex rounded-lg">전체 지역 선택</div>
                </div>
        `;
        if (mtxList.length < 1) {
            html += ``;
            this.lastSelection.uuid = "";
        } else {
            //마지막 선택했던 서버가 아직 있나?
            for (i = 0; i < mtxList.length; i++) {
                if (mtxList[i].mtxInfo.uuid == this.lastSelection.uuid) {
                    iLast = mtxList[i].uniq_seq;
                    break;
                }
            }
            if (iLast < 0) iLast = mtxList[0].uniq_seq;
            html += `<ul class="flex flex-row h-full">`;
            for (i = 0; i < mtxList.length; i++) {
                let mtxNick = mtxList[i].mtxInfo.id;
                let mtxD = mtxList[i].getMainTXDev();
                if (mtxD != null) {
                    if (mtxD.nick_name != "NONE") mtxNick = mtxD.nick_name;
                }                                 
        
                let stlStr = `background-color: ${COLOR_MULTITAB_NOR};`;
                let clsStr = "flex justify-center items-center h-full cursor-pointer px-[20px] first:rounded-tl-lg last:rounded-tr-lg";
                if (mtxList[i].uniq_seq == iLast) stlStr = `background-color: ${COLOR_MULTITAB_SEL};`;                
                html += `
                    <li class="${clsStr}" id='bm_mtxlist_${mtxList[i].uniq_seq}' style="${stlStr}">
                        ${mtxNick}
                    </li>
                `;
            }
            html += `</ul>`;
        }
        html += `</div>`;

        html += `
            <div class="h-[calc(100%-60px)] bg-[#343437] rounded-b-lg">
                <div class="flex flex-row h-full p-[8px]" id="bm_cts_main"></div>
            </div>
        `;

        html += `
        
        `;

        gDOM(DIVID_MAINCTS).innerHTML = html;

        if (iLast > 0) {
            for (i = 0; i < mtxList.length; i++) {
                gDOM("bm_mtxlist_" + mtxList[i].uniq_seq).addEventListener("click", this._selectMTX.bind(this, mtxList[i].uniq_seq, false));
            }
            this._selectMTX(iLast, true);
        }
        //console.log(gDOM(DIVID_MAINCTS).innerHTML);
    }

    _refreshModalLayout(searchWord, mtxConn, mtxConnMediaList) {
        if (searchWord === "") {
            this._showModalLayout(mtxConn, mtxConn.media_list);
            return;
        }

        let newMtxConnMediaList = [];
        newMtxConnMediaList = mtxConnMediaList.filter((m) => {
            return Base64.decode(m.b64_title).includes(searchWord);
        });
        this._showModalLayout(mtxConn, newMtxConnMediaList);
    }

    _hideModalLayout(state) {
        if (state == "brdMedia") {
            this._onControlBroadcast('stop');
        } else if (state == "brdMic") {
            this._onControlBroadcast('stop');
        } else {
            gDOM("modal-sounds").style.display = "none";
        }
    }

    _showModalLayout(mtxConn, mtxConnMediaList) {
        let html = "";
        let evtPairList = [];

        html += `
            <div id="modal-sounds" class="relative z-10" aria-labelledby="modal-title" role="dialog" aria-modal="true">
                <div class="fixed inset-0 transition-opacity bg-gray-500 bg-opacity-75"></div>
                <div class="fixed inset-0 z-10 w-screen overflow-y-auto">
                    <div class="flex items-end justify-center min-h-full p-4 text-center sm:items-center sm:p-0">
                        <div class="relative transform overflow-hidden rounded-[8px] shadow-xl transition-all w-[400px] h-[520px]">
                            <div class="flex items-center flex-col w-full h-full bg-[#343434] px-[12px] pt-[12px]">
                                <div class="flex justify-between items-center w-full h-[40px]">
                                    <div class="cursor-pointer basis-6/12" id="hideSoundModalBtn">
                                        <img src="../assets/images/closeOutline.png" />
                                    </div>
                                    <div class="text-lg text-center basis-6/12">음원</div>
                                    <div class="text-right basis-3/12">
                                        <span class="text-lg cursor-pointer" id="clusterAddBtn">확인</span>
                                    </div>
                                </div>
                                <div class="flex justify-between items-center w-full h-[44px] border-b">
                                    <div class="flex items-center justify-start w-full">
                                        <img src="../assets/images/magnify.png" />
                                        <div class="flex items-center justify-center w-full ml-[10px]">
                                            <input type="text" id="soundSearchInput" class="w-full input-reset" placeholder="검색" />
                                        </div>
                                    </div>
                                </div>
                                <div class="flex flex-col justify-start w-full h-[400px] overflow-auto custom-scrollbar">
        `;

        evtPairList.push({ id: "clusterAddBtn", fn: this._setModalData.bind(this) });

        for (let m of mtxConnMediaList) {
            if (m.idx > 4) {
                let tid = "bdc_media_select_" + m.idx;
                let tid2 = "bdc_media_down__" + m.idx;
                let mChk = "";
                let mlChk = "pinOFF";
                let i;

                let epf = { id: tid2, fn: this._onMediaDownload.bind(this, mtxConn.mtxInfo, m, tid2) };
                let epf2 = { id: tid, fn: this._setTempData.bind(this, tid, false, m) };
                for (i = 0; i < this.mSelectedSound.length; i++) {
                    if (this.mSelectedSound[i][0] === tid) {
                        mChk = "checked";
                        break;
                    }
                }

                if (this.localMedias !== null) {
                    for (i = 0; i < this.localMedias.length; i++) {
                        if (this.localMedias[i].remote_idx === m.idx) {
                            mlChk = "pinON";
                            break;
                        }
                    }
                }

                html += `
                    <div class="flex items-center justify-between w-full py-[8px]">
                        <div class="flex">
                            <label class="el-checkbox flex">
                                <input type="checkbox" id="${epf2.id}" name="check" value="option" ${mChk}/>
                                <span class="el-checkbox-style"></span>
                                <span class="truncate text-[14px] ml-[10px] w-[180px]">${Base64.decode(m.b64_title)}</span>
                            </label>
                        </div>
                        <div class="flex items-center justify-center">
                            <img id="${epf.id}" class="w-[24px] h-[24px] cursor-pointer" src="../assets/images/${mlChk}.png" />
                        </div>
                    </div>
                `;
                evtPairList.push(epf);
                evtPairList.push(epf2);
            }
        }

        html += `
                                </div>
                                <div class="flex items-center justify-between w-full text-[12px] text-[#c5c5c5]"></div>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        `;

        gDOM(DIVID_MAINCTS_MODAL).innerHTML = html;

        evtPairList.push({ id: "hideSoundModalBtn", fn: this._hideModalLayout.bind(this) });
        evtPairList.push({ id: "soundSearchInput" });

        for (let epf of evtPairList) {
            if (epf.id === "soundSearchInput")
                gDOM(epf.id).addEventListener("keydown", (e) => {
                    if (e.keyCode === 13) {
                        this._refreshModalLayout(e.target.value, mtxConn, mtxConnMediaList);
                    }
                });
            else gDOM(epf.id).addEventListener("click", epf.fn);
        }
    }

    _showModalLayout2() {
        let evtPairList = [];
        let html = "";
        let stMedia = "";
        let stGroup = "";
        let gCount = 0;

        for (let ms of this.broadcast.medias) {
            let parts, path;
            let nURI = ms.split("://");

            if (nURI[0] === "file") {
                for (let m of this.localMedias) {
                    if (m.local_uri === nURI[1]) {
                        stMedia += `<div id="ms__bdc_media_select_l_${m.remote_idx}" class="m-modal-play-list-item">${Base64.decode(
                            m.title
                        )}</div>`;
                    }
                }
            } else {
                parts = nURI[1].split("/");
                parts.shift();
                path = "/" + parts.join("/");

                for (let m of this.selectedMtx.media_list) {
                    if (Base64.decode(m.b64_local) === path) {
                        stMedia += `<div id="ms__bdc_media_select_nl_${m.idx}" class="m-modal-play-list-item">${Base64.decode(
                            m.b64_title
                        )}</div>`;
                    }
                }
            }
        }

        for (let gs of this.broadcast.groups) {
            for (let ga of this.selectedMtx.groupList) {
                if (gs === ga.idx) {
                    gCount++;
                    stGroup += `<div class="m-modal-list-sel-item-content">${ga.desc} (${ga.speakers.length})</div>`;
                }
            }
        }

        let txNick = "ERROR";
        if (this.broadcast.media_tx != null) {
            let myTX = this.selectedMtx.getMyTXDev();
            for (let txc of myTX.tx_channels) {
                if (txc.idx == this.broadcast.media_tx) {
                    txNick = txc.nick_name;
                    break;
                }
            }
        }

        let szVolMain = this._makeVolumeLayout("음원 출력", 0, 120);
        let szVolTX = this._makeVolumeLayout(txNick, 1, 120);

        html = `
            <div id="media_modal" class="m-modal">
                <div class="m-modal-content" style="width: 864px;">
                    <div class="m-modal-play" style="width: 46%;">
                        <div class="m-modal-play-title">음원방송</div>
                        <div class="m-modal-play-list">
                            <div class="custom-scrollbar">
                                <div class="m-modal-play-list-items">
                                    ${stMedia}
                                </div>
                            </div>
                        </div>
                        <div class="m-modal-play-btn">
                            <div id="m_modal_play_btn_prev">
                                <img src="../assets/images/play-back.png" />
                            </div>
                            <div id="m_modal_play_btn_next">
                                <img src="../assets/images/play-front.png" />
                            </div>
                        </div>
                        <div id="bdc_media_player" class="m-modal-play-control">
                            <audio controls>
                                <source src="" />
                            </audio>
                        </div>
                    </div>
                    <div  style="width: 30%; height: 100%; background-color: var(--penta-box-1st); padding: 12px;">
                        <div class="m-modal-list-sel">
                            <div class="m-modal-list-sel-list-title">출력(${gCount})</div>
                            <div class="m-modal-list-sel-list custom-scrollbar">
                                <div class="m-modal-list-sel-item">
                                    <div class="m-modal-list-sel-item-title">${this.selectedMtx.mtxInfo.id}</div>
                                    ${stGroup}
                                </div>
                            </div>
                        </div>
                        <div class="m-modal-list-btn">
                            <div id="hide_media_modal" class="m-modal-list-btn-close">방송 종료</div>
                        </div>
                    </div>
                    <div class="m-modal-list" style="display: flex;">
                        ${szVolMain}
                        ${szVolTX}
                    </div>
                </div>
            </div>
        `;

        gDOM(DIVID_MAINCTS_MODAL).innerHTML = html;

        evtPairList.push({ id: "hide_media_modal", fn: this._hideModalLayout.bind(this, "brdMedia") });
        evtPairList.push({ id: "m_modal_play_btn_prev", fn: this._onMediaPlayed.bind(this, false) });
        evtPairList.push({ id: "m_modal_play_btn_next", fn: this._onMediaPlayed.bind(this, true) });

        for (let epf of evtPairList) {
            gDOM(epf.id).addEventListener("click", epf.fn);
        }
        
        this.volumeSliders = new Array();        
        this.volumeSliders.push(new PentaVolumeSlide('main_vol_slide_0', 0, 1));        
        this.volumeSliders.push(new PentaVolumeSlide('main_vol_slide_1', 0, 1));        

        this.volumeSliders[0].Show();
        this.volumeSliders[0].SetValue(0);
        this.volumeSliders[0].SetOnChangeCallback(this._onVolumeSet.bind(this, 0));
        this.volumeSliders[1].Show();
        this.volumeSliders[1].SetValue(0);
        this.volumeSliders[1].SetOnChangeCallback(this._onVolumeSet.bind(this, 1));

        this.volumeSlideType = 10;
        this._getMainboardVolume();
        this._getHardwareInfo();        
    }

    _showModalLayout3() {
        let evtPairList = [];
        let html = "";
        let stGroup = "";
        let gCount = 0;

        for (let gs of this.broadcast.groups) {
            for (let ga of this.selectedMtx.groupList) {
                if (gs === ga.idx) {
                    gCount++;
                    stGroup += `<div class="i-modal-list-sel-item-content">${ga.desc} (${ga.speakers.length})</div>`;
                }
            }
        }

        let inputName = "MIC";
        let dom = gDOM("bdc_txch_" + this.broadcast.txch);

        if (dom != null) inputName = dom.innerText;        

        let txNick = "ERROR";
        if (this.broadcast.txch != null) {
            let myTX = this.selectedMtx.getMyTXDev();
            for (let txc of myTX.tx_channels) {
                if (txc.idx == this.broadcast.txch) {
                    txNick = txc.nick_name;
                    this.volumeSlideType = txc.order + 1;
                    break;
                }
            }
        }
        
        let szVolTX = this._makeVolumeLayout(txNick, this.volumeSlideType, 120);

        html = `
            <div id="mic_modal" class="i-modal">
                <div class="i-modal-content" style="width: 404px;">
                    <div class="i-modal-list" style="width: 244px;">
                        <div class="i-modal-list-title">
                            <div class="i-modal-list-title-main">방송</div>
                            <div class="i-modal-list-title-sub">입력 : ${inputName}</div>
                        </div>
                        <div class="i-modal-list-sel">
                            <div class="i-modal-list-sel-list-title">출력(${gCount})</div>
                            <div class="i-modal-list-sel-list custom-scrollbar">
                                <div class="i-modal-list-sel-item">
                                    <div class="i-modal-list-sel-item-title">${this.selectedMtx.mtxInfo.id}</div>
                                    ${stGroup}
                                </div>
                            </div>
                        </div>
                        <div id="i_modal_list_time" class="i-modal-list-time">진행시간 : 00:00</div>
                        <div id="bdc_mic_player" style="display: none;"></div>
                        <div class="i-modal-list-btn">
                            <div id="hide_mic_modal" class="i-modal-list-btn-close">방송 종료</div>
                        </div>
                    </div>
                    <div class="i-modal-list" style="width: 160px;">
                        ${szVolTX}
                    </div>
                </div>
            </div>
        `;

        gDOM(DIVID_MAINCTS_MODAL).innerHTML = html;

        evtPairList.push({ id: "hide_mic_modal", fn: this._hideModalLayout.bind(this, "brdMic") });

        for (let epf of evtPairList) {
            gDOM(epf.id).addEventListener("click", epf.fn);
        }

        this.volumeSliders = new Array();        
        this.volumeSliders.push(new PentaVolumeSlide(`main_vol_slide_${this.volumeSlideType}`, 0, 1));

        this.volumeSliders[0].Show();
        this.volumeSliders[0].SetValue(0);
        this.volumeSliders[0].SetOnChangeCallback(this._onVolumeSet.bind(this, this.volumeSlideType));
        this._getHardwareInfo();        
    }

    _setTempData(szDomID, bIsLocal, mediaObj) {
        for (let i = 0; i < this.mSelectedSound.length; i++) {
            if (this.mSelectedSound[i].includes(szDomID)) {
                this.mSelectedSound.splice(i, 1);
                return;
            }
        }
        this.mSelectedSound.push([szDomID, bIsLocal, mediaObj]);
    }

    _setModalData() {
        let i, j;
        let ms = null;

        for (j = this.broadcast.medias.length - 1; j >= 0; j--) {
            let bd = this.broadcast.medias[j];
            let nURI = bd.split("://");

            if (nURI[0] === "http") {
                this.broadcast.medias.splice(j, 1);
            }
        }

        for (i = 0; i < this.mSelectedSound.length; i++) {
            ms = this.mSelectedSound[i];
            this._onMediaSelect2(ms[1], ms[2]);
        }

        if (this._isBrdMedia()) {
            document.getElementById("show-sound-modal-btn").classList.add("custom-click");
            document.getElementById("show-sound-modal-btn").innerHTML = `음원(${this.mSelectedSound.length})`;
        } else {
            document.getElementById("show-sound-modal-btn").classList.remove("custom-click");
            document.getElementById("show-sound-modal-btn").innerHTML = `음원`;
        }

        this._hideModalLayout();
    }

    _checkLocalChime(mtxConn) {
        if (this.localMedias === null) {
            for (let m of mtxConn.media_list) {
                if (m.idx === 2 || m.idx === 3) {
                    this._onMediaDownload(mtxConn.mtxInfo, m);
                } else {
                    console.log("MAINTX - NO CHIME");
                }
            }
        } else {
            let isStartChime = false;
            let isEndChime = false;

            for (let m of this.localMedias) {
                if (m.seq === 1) isStartChime = true;
                else if (m.seq === 2) isEndChime = true;
            }

            if (!isStartChime || !isEndChime) {
                for (let m of mtxConn.media_list) {
                    if (m.idx === 2 && !isStartChime) this._onMediaDownload(mtxConn.mtxInfo, m);
                    if (m.idx === 3 && !isEndChime) this._onMediaDownload(mtxConn.mtxInfo, m);
                }
            }
        }
    }

    _selectMTX(uniq_seq, bForceRefresh = false) {        
        let tmtx = null;
        for (let emtx of this.mtxList) {
            if (emtx.uniq_seq == uniq_seq) {
                tmtx = emtx;
                break;
            }
        }
        if (tmtx == null) return;
        //console.log("_selectMTX: " + this.lastSelection.uuid + ", " + tmtx.mtxInfo.uuid);
        if (this.lastSelection.uuid == tmtx.mtxInfo.uuid) {
            if (!bForceRefresh) return;        
        }
        else {            
            this.grid_scroll_pos = 0; //MTX바꼈으면 스크롤 위치도 초기화
            //그룹 선택도 초기화 한다
            if (this.selectedMtx != null) {
                let totalSpk = [];
                for(let eg of this.broadcast.groups) {
                    let grpSpks = this.selectedMtx.getSpeakersOfGroup(eg);
                    for(let egs of grpSpks) {
                        if (totalSpk.indexOf(egs.idx) < 0) totalSpk.push(egs.idx);
                    }
                }
                if(totalSpk.length > 0) this._controlSpeakers(false, totalSpk);            
            }
            this.broadcast.groups = [];                                                        
        }

        let dom = gDOM(this.lastSelection.dom_id);
        if (dom != null) dom.style.backgroundColor = COLOR_MULTITAB_NOR;

        this.lastSelection.uuid = tmtx.mtxInfo.uuid;
        this.lastSelection.dom_id = "bm_mtxlist_" + uniq_seq;

        gDOM(this.lastSelection.dom_id).style.backgroundColor = COLOR_MULTITAB_SEL;

        this.connector.selectedMtx = tmtx;

        this._checkLocalChime(tmtx);
        this._refreshMTX(tmtx);
    }

    _onMediaDownload(mtxInfo, mediaInfo, szDomID = null) {
        let reqV = null;
        let data = null;
        // console.log(`WTF: ${mtxInfo.ip} - ${mtxInfo.uuid} - ${mediaInfo.idx}`);
        if (szDomID != null) {
            for (let lm of this.localMedias) {
                if (szDomID.split("__")[1] == lm.remote_idx) {
                    reqV = { act: "DEL_LOCAL_MEDIA", payload: lm };
                    this.funcCallNative(JSON.stringify(reqV));
                    document.getElementById(szDomID).src = "../assets/images/pinOFF.png";
                    console.log("Del local media called");
                    return;
                }
            }
        }

        data = {
            mtx_ip: mtxInfo.ip,
            mtx_uuid: mtxInfo.id,
            remote_uri: Base64.decode(mediaInfo.b64_local),
            remote_idx: mediaInfo.idx,
            title: mediaInfo.b64_title,
        };
        reqV = { act: "ADD_LOCAL_MEDIA", payload: data };
        this.funcCallNative(JSON.stringify(reqV));
        if (document.getElementById(szDomID) !== null) document.getElementById(szDomID).src = "../assets/images/pinON.png";
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

    _onControlBroadcast(szControl) {
        if (this.broadcast.state == BRDSTATE_IDLE) {
            if (this.selectedMtx == null) return;
            let dom = gDOM("bdc_autochime");
            let bAutoChime = dom.checked;            
            let bStartChimeOnly = false;
            if (szControl == 'start_chime_only') bStartChimeOnly = true;

            if (bAutoChime || bStartChimeOnly) {
                //시작/종료 차임 음원 찾기, idx=2, 3 으로 고정
                if (this.chimeMedia.length < 2) {
                    this.chimeMedia = [];
                    let tM = this._findMediaByIdx(2);
                    if (tM == null) {
                        this._showAlertModal("오류", "시작차임 음원이 없습니다.");
                        return;
                    }
                    this.chimeMedia.push(tM);
                    tM = this._findMediaByIdx(3);
                    if (tM == null) {
                        this._showAlertModal("오류", "종료차임 음원이 없습니다.");
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
            if (this.broadcast.medias.length > 0) {
                //미디어 재생이면 채널은 1번 채널로 고정
                if (!bStartChimeOnly) this.broadcast.txch = mediaTXC;
            } else {
                if (this.broadcast.txch == 0) {
                    this._showAlertModal("오류", "입력채널 또는 음원을 선택하세요");
                    return;
                }
            }
            if (this.broadcast.groups.length < 1) {
                this._showAlertModal("오류", "출력 그룹을 선택하세요");
                return;
            }

            if (this.broadcast.autoChime || bStartChimeOnly) {
                this.broadcast.state = BRDSTATE_CHIME_START_WAIT;
                if(bStartChimeOnly) this.broadcast.state = BRDSTATE_CHIME_START_ONLY;
                this._rest_controlBroadcast(true, mediaTXC, true);
            } else {
                this.broadcast.state = BRDSTATE_NORMAL_PLAYING;
                this._rest_controlBroadcast(true, this.broadcast.txch, true);
            }
        } else if (this.broadcast.state == BRDSTATE_MEDIA_PLAYING) {
            this._cancelMediaPlayer();
            if (this.broadcast.autoChime) {
                let dom = null;
                if (this._isBrdMedia()) {
                    dom = gDOM("hide_media_modal");
                } else {
                    dom = gDOM("hide_mic_modal");
                }
                dom.innerText = "종료 차임 재생중..";
                this.broadcast.state = BRDSTATE_CHIME_END_PLAYING;
                this._setMediaPlayer(this.chimeMedia[1]);
            } else this._onBroadcastDone(true);
        } else if (this.broadcast.state == BRDSTATE_NORMAL_PLAYING) {
            let bNeedMedia = false;
            //우선 MIC끄기
            for(let gs of this.broadcast.gpio_signals) {
                if (gs.pin == GPIOSIGNAL_MIC) gs.value = 0; //끄기
            }                    
            if (this.broadcast.autoChime) {
                bNeedMedia = true;
                let dom = gDOM("hide_mic_modal");
                if (dom != null) dom.innerText = "종료 차임 재생중..";
                if (this.broadcast.txch != this.broadcast.media_tx) {
                    this.broadcast.state = BRDSTATE_EXCHANGE_TX2MEDIA;
                    this._rest_exchangeBroadcastTX(this.broadcast.media_tx);
                } else { //채널이 같다! 즉 MIC플레이중!
                    this.broadcast.state = BRDSTATE_CHIME_END_PLAYING;
                    this._setMediaPlayer(this.chimeMedia[1]);
                }
            } else {
                let dom = gDOM("hide_mic_modal");
                if (dom != null) dom.innerText = "종료 차임 재생중..";
                if (this.broadcast.edChime) {
                    bNeedMedia = true;
                    this.broadcast.state = BRDSTATE_CHIME_END_PLAYING;
                    this._setMediaPlayer(this.chimeMedia[1]);
                } else {
                    this._onBroadcastDone(true);
                }
            }
            if (bNeedMedia) {                            
                for(let gs of this.broadcast.gpio_signals) {
                    if (gs.pin == GPIOSIGNAL_MEDIA) gs.value = 0; //켜기 --> 반대다
                }                    
            }
            let jsGpio = { act: 'SET_GPIO_VALUES', payload: {pins: this.broadcast.gpio_signals} };        
            console.log("GPIO1: " + JSON.stringify(jsGpio.payload));
            this.funcCallNative(JSON.stringify(jsGpio));
        }
    }

    _onGroupSelectAll(mtxConn) {
        if (mtxConn == null) return;
        this._selectAllMTXGroup(mtxConn, true);
    }

    _selectAllMTXGroup(mtxConn) {
        let bSelectAll = true;
        //하나라도 선택된게 있으면 해제
        for (let g of mtxConn.groupList) {
            if (g.idx < 2) continue;
            if(this.broadcast.groups.indexOf(g.idx) >= 0) {
                bSelectAll = false;
                break;
            }
        }

        let spkList = [];
        for (let g of mtxConn.groupList) {
            if (g.idx < 2) continue;            
            let tid = "bdc_group_list_" + g.idx;          
            let tSpks = null;
            if (bSelectAll) {                  
                if(this.broadcast.groups.indexOf(g.idx) < 0) tSpks = this._onGroupSelect(tid, g.idx, true);                
            }            
            else {
                if(this.broadcast.groups.indexOf(g.idx) >= 0) tSpks = this._onGroupSelect(tid, g.idx, true);                
            }
            if (tSpks != null) spkList = [...spkList, ...tSpks];
        }
        this._controlSpeakers(bSelectAll, spkList);        
    }

    _onGroupSelect(szDomID, idx, isAll) {
        //this.broadcast = {groups: [], txch: 0, medias: [], state: 0};
        if (this.selectedMtx == null) return null;
        if (this.broadcast.state != BRDSTATE_IDLE) return null;
        let dom = gDOM(szDomID);
        if (dom == null) {
            console.log("DOM is null: " + szDomID);
            return null;
        }
        let exIdx = this.broadcast.groups.indexOf(idx);
        let grpSpks = this.selectedMtx.getSpeakersOfGroup(idx);
        let arrSpks = [];
        for(let egs of grpSpks) arrSpks.push(egs.idx);
        
        let bIsEnable = false;
        if (exIdx < 0) {
            this.broadcast.groups.push(idx);
            dom.classList.add("custom-click");
            bIsEnable = true;
        } else {
            this.broadcast.groups.splice(exIdx, 1);
            dom.classList.remove("custom-click");
        }   
        
        this.selectedMtx.onGroupSelect(idx, bIsEnable);
        if (isAll != true) this._controlSpeakers(bIsEnable, arrSpks);        
        this._onSelectionChange();
        return arrSpks;
    }

    _onTXChannelSelect(szDomID, idx) {
        //this.broadcast = {groups: [], txch: 0, medias: [], state: 0};
        if (this.broadcast.state != BRDSTATE_IDLE) return;

        let dom;
        if (this.broadcast.txch === idx) {            
            dom = gDOM("bdc_txch_" + this.broadcast.txch);
            if (dom !== null) dom.classList.remove("custom-click");    
            this.broadcast.txch = 0;
        }
        else {
            this._disableMediaSelection();
            let dom;
            if (this.broadcast.txch > 0) {
                dom = gDOM("bdc_txch_" + this.broadcast.txch);
                if (dom !== null) dom.classList.remove("custom-click");    
                gDOM("select-mic").classList.remove("custom-click"); 
            }
            dom = gDOM(szDomID);
            dom.classList.add("custom-click");
            this.broadcast.txch = idx;
        }
        this._onSelectionChange();
    }

    _genMediaURI(bIsLocal, mediaObj) {
        let res = null;
        if (bIsLocal) {
            res = "file://" + mediaObj.local_uri;
        } else {
            res = `http://${this.selectedMtx.mtxInfo.ip}${Base64.decode(mediaObj.b64_local)}`;
        }
        return res;
    }

    _onMediaSelect(szDomID, bIsLocal, mediaObj) {
        //this.broadcast = {groups: [], txch: 0, medias: [], state: 0};
        //console.log("DomID: " + szDomID);
        //console.log("Media: " + JSON.stringify(mediaObj));
        if (this.broadcast.state != BRDSTATE_IDLE) return;
        let uri = this._genMediaURI(bIsLocal, mediaObj);
        let exIdx = -1;
        for (let i = 0; i < this.broadcast.medias.length; i++) {
            if (this.broadcast.medias[i] == uri) {
                exIdx = i;
                break;
            }
        }
        let dom;
        //음원을 선택하면 입력채널은 선택할 수 없다
        if (this.broadcast.txch > 0) {
            dom = gDOM("bdc_txch_" + this.broadcast.txch);
            if (dom !== null) dom.classList.remove("custom-click");
            // dom.classList.add("td_click");
            this.broadcast.txch = 0;
            gDOM("select-mic").classList.remove("custom-click");
        }
        dom = gDOM(szDomID);
        if (exIdx < 0) {
            this.broadcast.medias.push(uri);
            dom.classList.add("custom-click");
        } else {
            this.broadcast.medias.splice(exIdx, 1);
            dom.classList.remove("custom-click");
        }
        this._onSelectionChange();
    }

    _onMediaSelect2(bIsLocal, mediaObj) {
        //this.broadcast = {groups: [], txch: 0, medias: [], state: 0};
        if (this.broadcast.state != BRDSTATE_IDLE) return;
        let uri = this._genMediaURI(bIsLocal, mediaObj);

        let dom;
        //음원을 선택하면 입력채널은 선택할 수 없다
        if (this.broadcast.txch > 0) {
            dom = gDOM("bdc_txch_" + this.broadcast.txch);
            if (dom !== null) dom.classList.remove("custom-click");
            this.broadcast.txch = 0;
            gDOM("select-mic").classList.remove("custom-click");
        }

        this.broadcast.medias.push(uri);
    }

    _startSlice() {
        if (this.slideInterval) clearInterval(this.slideInterval);
        let slides = document.getElementsByClassName("carousel-item");

        function addActive(slide) {
            slide.classList.add("active");
        }

        function removeActive(slide) {
            slide.classList.remove("active");
        }

        if (slides == null || slides.length < 1) return;        
        addActive(slides[0]);
        if (slides.length > 1) {
            this.slideInterval = setInterval(function () {
                for (let i = 0; i < slides.length; i++) {                    
                    if (slides[i].classList.contains("active")) {
                        setTimeout(removeActive, 350, slides[i]);
                        if (i + 1 == slides.length) addActive(slides[0]);
                        else addActive(slides[i + 1]);
                        break;
                    }
                }
            }, 3000);
        }
    }

    _refreshMTX(mtxConn) {
        let mtx = mtxConn.getMainTXDev();
        let uartStates = this.generateUARTState(mtxConn);
        let brdCountList = this.generateBRDState(mtxConn);
        let epf = null;

        this.selectedMtx = mtxConn;

        let mtxNick = mtx.nick_name;
        if (mtxNick == "NONE") mtxNick = mtx.name;

        let evtPairList = [];

        let szState = `
            <aside class="grid flex-none w-[200px] content-between">
                <div class="flex flex-col justify-center h-[100px] rounded-[8px] p-[12px] bg-[#232326]">
                    <div class="flex items-center justify-center">
                        <span class="text-[18px] font-bold">${mtxNick}</span>
                    </div>
                    <div class="flex flex-row justify-between mt-[8px]">
                        <div
                            class="w-[82px] h-[32px] inline-flex items-center justify-center text-[14px] bg-[#${
                                mtx.state === "CONNECTED" ? "2efe46" : "343437"
                            }] rounded-[32px] text-[#222222]"
                        >
                            DANTE
                        </div>
                        <div
                            class="w-[82px] h-[32px] inline-flex items-center justify-center text-[14px] bg-[#28D8E6] rounded-[32px] text-[#959595]"
                        >
                            LINK
                        </div>
                    </div>
                </div>
        `;

        let szEmrList = '';
        for (let ee of uartStates) {
            szEmrList += `
                <div class="carousel-item flex items-center justify-center flex-col bg-[${ee.bgColor}] rounded-[8px]">
                    <div class="flex items-center justify-center text-[14px]">${ee.name}</div>
                    <div class="flex items-center justify-center text-[18px] text-[${ee.txtColor}]">${ee.state}</div>
                </div>
            `;
        }

        szState += `
            <div class="rounded-[8px] p-[12px] bg-[#232326]">
                <div class="flex items-center justify-center">
                    <span class="text-[18px] font-bold">화재 수신기</span>
                </div>
                <div class="flex flex-row justify-center mt-[8px]">
                    <div class="carousel w-[176px] h-[68px]">
                        ${szEmrList}
                    </div>
                </div>
            </div>
        `;

        szState += `
            <div class="h-[380px] rounded-[8px] p-[12px] bg-[#232326]">
                <div class="flex items-center justify-center">
                    <span class="text-[18px] font-bold">방송정보</span>
                </div>
                <div class="flex flex-col justify-evenly h-[170px] mt-[8px] bg-[#343437] rounded-[8px] px-[20px] py-[10px]">
        `;

        for (let i = 0; i < brdCountList.length; i++) {
            if (brdCountList[i].title === "긴급방송" && brdCountList[i].count === 0) continue;            
            let clsStr = "flex justify-between rounded-[8px] px-[10px]";
            if (brdCountList[i].count > 0) clsStr += " bg-[" +  brdCountList[i].szColor + "]";
            szState += `
                <div class="${clsStr}">
                    <div>${brdCountList[i].title}</div>
                    <div>${brdCountList[i].count}</div>
                </div>
            `;
        }

        szState += `
            </div>
        `;

        szState += `
            <div class="mt-[8px]">
                <button
                    id="bdc_startstop"
                    class="w-full h-[52px] inline-flex items-center justify-center text-[18px] bg-[#454548] rounded-[8px]"
                >
                    방송 대기
                </button>
            </div>
        `;

        szState += `
            <div class="flex flex-row justify-between mt-[8px]">
                <button
                    id ="bdc_chime_st" 
                    class="w-[82px] h-[44px] inline-flex items-center justify-center text-[14px] bg-[#454548] rounded-[8px] ${
                        this.broadcast.stChime ? "custom-click" : ""
                    }"
                >
                    시작 차임
                </button>
                <button
                    id ="bdc_chime_ed" 
                    class="w-[82px] h-[44px] inline-flex items-center justify-center text-[14px] bg-[#454548] rounded-[8px] ${
                        this.broadcast.edChime ? "custom-click" : ""
                    }"
                >
                    종료 차임
                </button>
            </div>
        `;

        evtPairList.push({ id: "bdc_chime_st", fn: this._onChime.bind(this, "bdc_chime_st") });
        evtPairList.push({ id: "bdc_chime_ed", fn: this._onChime.bind(this, "bdc_chime_ed") });

        szState += `
            <div class="flex justify-between items-center mt-[8px]">
                <div class="flex justify-center text-sm w-[82px]">자동 차임</div>
                <div class="flex justify-center w-[82px]">
                    <label class="el-switch">
                        <input id="bdc_autochime" type="checkbox" name="switch" ${this.broadcast.autoChime ? "checked" : ""}/>
                        <span class="el-switch-style"></span>
                    </label>
                </div>
            </div>
        `;

        szState += `
                </div>
            </aside>
        `;

        evtPairList.push({ id: "bdc_startstop", fn: this._onControlBroadcast.bind(this, "bdc_startstop") });
        evtPairList.push({ id: "bdc_autochime", fn: this._autoChime.bind(this) });

        let szGroup = `
            <section class="flex-auto ml-[12px] mr-[8px] rounded-[8px]">
                <div class="h-[44px] mb-[8px]">
                    <div id="bdc_out_group_all" class="flex justify-center items-center cursor-pointer w-[82px] h-full bg-[#232326] rounded-[8px] text-[14px]">
                        전체 선택
                    </div>
                </div>
                <div class="h-[calc(100%-44px-8px)] px-[12px] py-[8px] bg-[#232326] rounded-[8px] overflow-auto custom-scrollbar" id="grp_grid_main">
                    <div class="grid gap-[12px]" style="grid-template-columns: repeat(auto-fill, minmax(150px, 1fr));">
        `;

        evtPairList.push({ id: "bdc_out_group_all", fn: this._onGroupSelectAll.bind(this, mtxConn) });

        for (let grp of mtxConn.groupList) {
            let tid = "bdc_group_list_" + grp.idx;
            let gSelected = "";            
            let szSelectable = "cursor-pointer";
            let szTextStyle = "";
            let szBrdState = "";

            if (this.broadcast.groups.length > 0) {
                for (let g of this.broadcast.groups) {
                    if (g === grp.idx) gSelected = "custom-click";
                }                
            }
            
            //장치는 잘 연결되어 있는지 확인한다
            let gState = mtxConn.getGroupSpeakersConnState(grp.idx);
            if (gState != "OK") szSelectable = "";

            //해당 그룹이 방송중인지 확인한다
            let brdState = mtxConn.getOutputStateString(grp.idx);
            //console.log("state " + grp.idx + ": " + brdState);
            if (brdState != 'IDLE') {
                szSelectable = "";
                szBrdState = `<div class="flex items-center justify-center justify-center text-center bg-[#3fa2f3] rounded-[8px]">방송중(${brdState})</div>`;
            }
            else {
                //방송중이 아니면 다른 사람이 선택 중인지 확인한다
                let enStr = mtxConn.getSpeakerEnabledString(grp.idx);
                if (enStr != null) {
                    szSelectable = "";
                    szBrdState = `<div class="flex items-center justify-center justify-center text-center bg-[#3fa2f3] rounded-[8px]">준비중(${enStr})</div>`;
                }
            }

            if (szSelectable.length <= 0) {
                szTextStyle = "style='color: #959595;'";
            }

            szGroup += `
                <div id="${tid}" class="h-[78px] p-[12px] bg-[#343437] rounded-[8px] ${szSelectable} ${gSelected}">
                    <div class="flex items-center justify-center text-center h-[22px]" ${szTextStyle}>${grp.desc}(${grp.speakers.length})</div>
                    ${szBrdState}
                </div>
            `;

            if (szSelectable.length > 0) {
                epf = { id: tid, fn: this._onGroupSelect.bind(this, tid, grp.idx, false) };
                evtPairList.push(epf);
            }
        }

        szGroup += `
                    </div>
                </div>
            </section>
        `;

        let myTX = mtxConn.getMyTXDev();
        let onSound = "";
        let onMIC = "";
        let onSoundCount = 0;

        if (this._isBrdMedia()) {
            for (let m of this.broadcast.medias) {
                let nURI = m.split("://");
                if (nURI[0] !== "file") {
                    onSound = "custom-click";
                    onSoundCount++;
                }
            }
        }
        else if (this.broadcast.txch === myTX.tx_channels[0].idx) onMIC = "custom-click";

        let scTXCMedia = `
            <aside class="flex-none w-[160px]">
                <div class="flex justify-center items-center h-[44px] mb-[8px] rounded-[8px] text-[18px] font-bold">입력</div>
                    <div class="h-[calc(100%-44px-8px)] px-[8px] bg-[#232326] rounded-[8px] overflow-auto custom-scrollbar" id="input_list_main">
                        <div class="pt-[8px]">
                            <button
                                id="show-sound-modal-btn"
                                class="w-full h-[52px] ${onSound} inline-flex items-center justify-center text-[14px] bg-[#343437] rounded-[8px]"
                            >
                                음원 ${onSoundCount > 0 ? "(" + onSoundCount + ")" : ""}
                            </button>
                        </div>
                        <div class="pt-[8px]">
                            <button id="select-mic"
                                class="w-full h-[52px] ${onMIC} inline-flex items-center justify-center text-[14px] bg-[#343437] rounded-[8px]">
                                마이크
                            </button>
                        </div>
        `;
        
        evtPairList.push({ id: "show-sound-modal-btn", fn: this._showModalLayout.bind(this, mtxConn, mtxConn.media_list) });
        evtPairList.push({ id: "select-mic", fn: this._selectMIC.bind(this, mtxConn) });

        for (let txc of myTX.tx_channels) {
            if (txc.order === 0) continue;
            let tid = "bdc_txch_" + txc.idx;
            let tChk = "";
            if (this.broadcast.txch === txc.idx) tChk = "custom-click";
            scTXCMedia += `
                <div class="pt-[8px]">
                    <button
                        id="${tid}"
                        class="w-full h-[52px] inline-flex items-center justify-center text-[14px] bg-[#343437] rounded-[8px] ${tChk}"
                    >
                        ${txc.nick_name}
                    </button>
                </div>
            `;
            epf = { id: tid, fn: this._onTXChannelSelect.bind(this, tid, txc.idx) };
            evtPairList.push(epf);
        }

        this.localMediaDOM_IDs = [];
        if (this.localMedias != null) {
            scTXCMedia += `<div class="pt-[8px] border-b-[3px] border-dashed border-[#757575]"></div>`;
            for (let m of this.localMedias) {
                if (m.remote_idx < 5) continue; //차임은 표시하지 않는다
                let tid = "bdc_media_select_local_" + m.uniq_seq + "_" + m.seq;
                this.localMediaDOM_IDs.push(tid);
                let epf = { id: tid, fn: this._onMediaSelect.bind(this, tid, true, m) };

                let gSelected = "";

                if (this.broadcast.medias.length > 0) {
                    for (let m2 of this.broadcast.medias) {
                        let nURI = m2.split("://");
                        if (nURI[0] !== "file") continue;
                        if (m.local_uri === nURI[1]) gSelected = "custom-click";
                    }
                }

                scTXCMedia += `
                    <div class="pt-[8px]">
                        <button
                            id="${epf.id}"
                            class="w-full h-[52px] inline-flex items-center justify-center text-[14px] bg-[#343437] rounded-[8px] ${gSelected}"
                        >
                            ${Base64.decode(m.title)}
                        </button>
                    </div>
                `;
                evtPairList.push(epf);
            }
        }

        scTXCMedia += `
                </div>
            </aside>
        `;

        let html = `
            ${szState} 
            ${szGroup}
            ${scTXCMedia}
        `;

        gDOM("bm_cts_main").innerHTML = html;
        let dom = gDOM("grp_grid_main");
        dom.scrollTop = this.grid_scroll_pos;
        dom.onscroll = this._onGridScroll.bind(this);
        dom = gDOM("input_list_main");
        dom.scrollTop = this.input_scroll_pos;
        dom.onscroll = this._onInputScroll.bind(this);

        this._startSlice();
        for (let epf of evtPairList) {
            gDOM(epf.id).addEventListener("click", epf.fn);
        }
        this.preReadyState = false;
        this._onSelectionChange();
    }

    generateUARTState(mtxConn) {
        let res = new Array();                
        //console.log("generateUARTState");
        //발생한 방송이 하나라도 있으면 연결 상태는 보여주지 않는다
        let bShowConnState = true;
        for(let eu of mtxConn.uart_list) {
            for(let eg of eu.emrGroups) {
                if (eg.extra1 == 'AUTO' || eg.extra1 == 'MANUAL') {      
                    bShowConnState = false;
                    break;
                }
            }
        }

        for(let eu of mtxConn.uart_list) {
            let iCnt = 0;
            //console.log("generateUARTState: " + JSON.stringify(eu));
            for(let eg of eu.emrGroups) {
                //if (eu.port > 1) console.log(eg.extra1);
                if (eg.extra1 == 'AUTO' || eg.extra1 == 'MANUAL') {      
                    //console.log("Occured state: " + eg.extra1);
                    let tArr = {name: eu.name, state: eg.name, txtColor: '#FFFFFF', bgColor: '#FF7F00'};
                    if (eu.port == 1) {
                        tArr.bgColor = '#FF0000';
                    }
                    res.push(tArr);
                    iCnt++;
                }
            }

            if (iCnt < 1 && bShowConnState) {
                let tArr = {name: eu.name, state: '정상', txtColor: '#FFFFFF', bgColor: '#343437'};
                let spl = eu.info.split('__');
                            
                if (spl[0] != 'GPIO') { //2 = GPIO = 항상 정상
                    if (eu.state == "DISCONNECTED") {
                        tArr.state = "연결 안됨";
                        tArr.txtColor = '#FF0000';
                    }
                    else if (eu.state == "SIGNAL_RELAY_DIS") {
                        tArr.state = "연동 정지";
                        tArr.txtColor = '#00FF00';
                    }                    
                }    
                res.push(tArr);                       
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
                if (cb.state == "PLAYING") {
                    let bCate = mtxConn.getBCastCategory(cb.tx_id, cb.type_idx); //MAN, BGM, EMR, REM, PCB
                    switch (bCate) {
                        case "MAN":
                            main_count++;
                            break;
                        case "EMR":
                            emg_count++;
                            break;
                        case "BGM":
                            bgm_count++;
                            break;
                        case "REM":
                            remote_count++;
                            break;
                        case "PCB":
                            etc_count++;
                            break;
                    }
                }
            }
        }
        res.push({ title: "일반방송", count: main_count, szColor: '#2b86ff' });
        res.push({ title: "긴급방송", count: emg_count, szColor: '#ff4040' });
        res.push({ title: "BGM방송", count: bgm_count, szColor: '#70ad47' });
        res.push({ title: "PC방송", count: etc_count, szColor: '#002060' });
        res.push({ title: "리모트방송", count: remote_count, szColor: '#ffc000' });
        return res;
    }

    _rest_controlBroadcast(bStart, txch, bResetOutput) {
        if (this.selectedMtx == null) return;
        if (bStart) {
            let payload = { control: "start", txc: txch, groups: this.broadcast.groups };
            this.selectedMtx.rest_call(
                "control_broadcast",
                payload,
                null,
                function (bRes, jsRecv, callParam) {
                    if (bRes != true || jsRecv.res != true) {
                        this._onBroadCastStartFailed(jsRecv.error);
                        return;
                    }
                    console.log("Control res: " + JSON.stringify(jsRecv));
                    this.broadcast.broadcast_idx = jsRecv.payload.broadcast_idx;
                    this.broadcast.gpio_signals = [{pin: GPIOSIGNAL_ONAIR, value: 1}];
                    let dom = null;
                    if (this._isBrdMedia()) {
                        this._showModalLayout2();
                        dom = gDOM("hide_media_modal");                        
                    } else {
                        this._showModalLayout3();
                        this._startMicBrdTimer();
                        dom = gDOM("hide_mic_modal");                        
                    }

                    if (this._isBrdMedia() 
                        || this.broadcast.state == BRDSTATE_CHIME_START_WAIT
                        || this.broadcast.state == BRDSTATE_CHIME_START_ONLY ) {
                        this.broadcast.gpio_signals.push({pin: GPIOSIGNAL_MEDIA, value: 0}); //반대다
                    }
                    else {
                        if (txch === this.selectedMtx.getMyMediaChannel()) {
                            this.broadcast.gpio_signals.push({pin: GPIOSIGNAL_MIC, value: 1});
                        }                
                    }

                    let jsGpio = { act: 'SET_GPIO_VALUES', payload: {pins: this.broadcast.gpio_signals} };        
                    console.log("GPIO2: " + JSON.stringify(jsGpio.payload));
                    this.funcCallNative(JSON.stringify(jsGpio));
            
                    if (this.broadcast.state == BRDSTATE_CHIME_START_WAIT || this.broadcast.state == BRDSTATE_CHIME_START_ONLY) {
                        //시작 차임 재생 대기
                        dom.innerText = "시작 차임 재생중...";
                        if (this.broadcast.state != BRDSTATE_CHIME_START_ONLY) this.broadcast.state = BRDSTATE_CHIME_START_PLAYING;                        
                        this._setMediaPlayer(this.chimeMedia[0]);
                    } else if (this.broadcast.state == BRDSTATE_NORMAL_PLAYING) {
                        //즉시 재생
                        dom.innerText = "방송 중지";
                        if (this.broadcast.medias.length > 0) {
                            this.mediaPlayCount = 0;
                            this.broadcast.state = BRDSTATE_MEDIA_PLAYING;
                            this._setMediaPlayer(this.broadcast.medias[0]);
                        }
                    }
                }.bind(this)
            );
        } else {
            let payload = { control: "stop", broadcast_idx: this.broadcast.broadcast_idx };
            this.selectedMtx.rest_call(
                "control_broadcast",
                payload,
                null,
                function (bRes, jsRecv, callParam) {
                    if (bRes != true || jsRecv.res != true) {
                        this._onBroadCastStartFailed(jsRecv.error);
                        return;
                    }
                    this._finalizeBroadcast(bResetOutput);
                }.bind(this)
            );
        }
    }

    _rest_exchangeBroadcastTX(txch) {
        let payload = { control: "exchange_tx", txc: txch, broadcast_idx: this.broadcast.broadcast_idx };
        this.selectedMtx.rest_call(
            "control_broadcast",
            payload,
            null,
            function (bRes, jsRecv, callParam) {
                if (bRes != true || jsRecv.res != true) {
                    this._onBroadCastStartFailed(jsRecv.error);
                    return;
                }
                if (this.broadcast.state == BRDSTATE_EXCHANGE_TX2NORMAL) {
                    this.broadcast.state = BRDSTATE_NORMAL_PLAYING;
                } else if (this.broadcast.state == BRDSTATE_EXCHANGE_TX2MEDIA) {
                    this.broadcast.state = BRDSTATE_CHIME_END_PLAYING;
                    this._setMediaPlayer(this.chimeMedia[1]);
                }
            }.bind(this)
        );
    }

    _setMediaPlayer(uri) {
        let dom = null;
        if (this._isBrdMedia()) dom = gDOM("bdc_media_player");
        else dom = gDOM("bdc_mic_player");

        console.log("Set media: " + uri);
        if (uri == null) {
            if (this._isBrdMedia()) {
                dom.innerHTML = `<audio controls ><source src=""></audio>`;
                gDOM("media_modal").style.display = "none";
            } else {
                dom.innerHTML = `<audio controls ><source src=""></audio>`;
                if (this.broadcast.state === BRDSTATE_FINISHING) {
                    gDOM("mic_modal").style.display = "none";
                }
            }
        } else {
            dom.innerHTML = `<audio controls autoplay style="width: 80%; height: 20px;" id="bdc_media_play_actl"><source src="${uri}"></audio>`;
            dom = gDOM("bdc_media_play_actl");
            dom.addEventListener("ended", this._onMediaPlayed.bind(this, null));
        }

        if (this._isBrdMedia()) {
            let pElement = document.querySelector(".m-modal-play-list-items");
            let tElements = pElement.querySelectorAll(".custom-focus");

            tElements.forEach((el) => {
                el.classList.remove("custom-focus");
            });

            if (this.broadcast.state === BRDSTATE_CHIME_START_PLAYING || this.broadcast.state === BRDSTATE_CHIME_END_PLAYING) return;
            if (uri === null || uri === undefined) return;

            let parts, path;
            let nURI = uri.split("://");

            if (nURI[0] === "file") {
                for (let m of this.localMedias) {
                    if (m.local_uri === nURI[1]) {
                        gDOM(`ms__bdc_media_select_l_${m.remote_idx}`).classList.add("custom-focus");
                    }
                }
            } else {
                parts = nURI[1].split("/");
                parts.shift();
                path = "/" + parts.join("/");

                for (let ms of this.selectedMtx.media_list) {
                    if (Base64.decode(ms.b64_local) === path) {
                        gDOM(`ms__bdc_media_select_nl_${ms.idx}`).classList.add("custom-focus");
                    }
                }
            }
        }
    }

    _cancelMediaPlayer() {
        let dom = gDOM("bdc_media_play_actl");
        if (dom != null) dom.pause();
        // this._setMediaPlayer(null);
    }

    _onMediaPlayed(isNext) {
        console.log("On media played");
        let dom = null;
        if (this._isBrdMedia()) {
            dom = gDOM("hide_media_modal");
        } else {
            dom = gDOM("hide_mic_modal");
        }
        if (this.broadcast.state == BRDSTATE_CHIME_START_PLAYING) {
            //시작차임 재생 완료
            dom.innerText = "방송 중지";
            if (this.broadcast.medias.length > 0) {
                this.mediaPlayCount = 0;
                this.broadcast.state = BRDSTATE_MEDIA_PLAYING; //미디어 재생 대기
                this._setMediaPlayer(this.broadcast.medias[0]);
            } else {
                this._setMediaPlayer(null);
                if (this.broadcast.txch != this.broadcast.media_tx) {
                    for(let gs of this.broadcast.gpio_signals) {
                        if (gs.pin == GPIOSIGNAL_MEDIA) gs.value = 1; //미디어 방송중 끄기 --> 1이 끄는거
                    }
                    //채널 변경
                    this.broadcast.state = BRDSTATE_EXCHANGE_TX2NORMAL;
                    this._rest_exchangeBroadcastTX(this.broadcast.txch);
                } else { //차임 재생이 완료되었는데 방송 채널이 미디어다?? 이건 MIC방송!
                    this.broadcast.state = BRDSTATE_NORMAL_PLAYING;
                    this.broadcast.gpio_signals.push({pin: GPIOSIGNAL_MIC, value: 1}); //마이크 방송중 켜기
                }
                let jsGpio = { act: 'SET_GPIO_VALUES', payload: {pins: this.broadcast.gpio_signals} };      
                console.log("GPIO3: " + JSON.stringify(jsGpio.payload));  
                this.funcCallNative(JSON.stringify(jsGpio));
            }
        } else if (this.broadcast.state == BRDSTATE_MEDIA_PLAYING) {
            //미디어 재생 완료
            if (isNext === null) {
                this.mediaPlayCount++;
                if (this.mediaPlayCount >= this.broadcast.medias.length) {
                    if (this.broadcast.autoChime) {
                        dom.innerText = "종료 차임 재생중...";
                        this.broadcast.state = BRDSTATE_CHIME_END_PLAYING; //종료차임 재생 대기
                        this._setMediaPlayer(this.chimeMedia[1]);
                    } else this._onBroadcastDone(true);
                } else {
                    this._setMediaPlayer(this.broadcast.medias[this.mediaPlayCount]);
                }
            } else {
                if (isNext) {
                    let lastIdx = this.broadcast.medias.length - 1;
                    if (lastIdx === this.mediaPlayCount) this.mediaPlayCount = lastIdx;
                    else this.mediaPlayCount++;
                } else if (!isNext) {
                    if (this.mediaPlayCount !== 0) this.mediaPlayCount--;
                } else {
                    this.mediaPlayCount++;
                }
                this._setMediaPlayer(this.broadcast.medias[this.mediaPlayCount]);
            }
        } else if (this.broadcast.state == BRDSTATE_CHIME_END_PLAYING || this.broadcast.state == BRDSTATE_CHIME_START_ONLY) {
            //종료차임 재생 완료
            this._onBroadcastDone(true);
        }
    }

    _onBroadcastDone(bCallRest) {
        if (!this._isBrdMedia()) {
            this.totalSeconds = 0;
            clearInterval(this.intervalId);
            gDOM("i_modal_list_time").textContent = "진행시간 : 00:00";
        }

        let bReset = true;
        if (this.broadcast.state == BRDSTATE_CHIME_START_ONLY) bReset = false;
        this.broadcast.state = BRDSTATE_FINISHING;
        this._setMediaPlayer(null);
        if (bCallRest) this._rest_controlBroadcast(false, this.broadcast.txch, bReset);
        else this._finalizeBroadcast(bReset);
    }

    _isBrdMedia() {
        if (this.broadcast.medias.length > 0) return true;
        else return false;
    }

    _onChime(szId) {
        if (szId != "bdc_chime_st") return;
        if (this.preReadyState != true) return;
        if (this.broadcast.autoChime) return;
        this._onControlBroadcast('start_chime_only');
    }

    _autoChime() {
        if (gDOM("bdc_autochime").checked) {
            this.broadcast.autoChime = true;
            gDOM("bdc_chime_st").classList.remove("custom-click"); // 시작
            //gDOM("bdc_chime_ed").classList.add("custom-click"); // 종료
        } else {
            this.broadcast.autoChime = false;
            if (this.preReadyState) gDOM("bdc_chime_st").classList.add("custom-click"); // 시작
            //gDOM("bdc_chime_ed").classList.remove("custom-click"); // 종료
        }
    }

    _startMicBrdTimer() {
        this.intervalId = setInterval(() => {
            this.totalSeconds++;

            let minutes = Math.floor(this.totalSeconds / 60);
            let seconds = this.totalSeconds % 60;

            let displayMinutes = minutes < 10 ? "0" + minutes : minutes;
            let displaySeconds = seconds < 10 ? "0" + seconds : seconds;

            gDOM("i_modal_list_time").textContent = "진행시간 : " + displayMinutes + ":" + displaySeconds;
        }, 1000);
    }

    //방송 목록이 업데이트 됨 - 내꺼는 잘 되고 있는지 확인
    _checkBroadcastList() {
        if (this.broadcast.state == BRDSTATE_IDLE) return;        
        if (this.selectedMtx == null) return;

        let bMineStopped = false;
        for (let cb of this.selectedMtx.broadcast_list) {
            if (cb.idx != this.broadcast.broadcast_idx) continue;
            if (cb.state != "PLAYING") bMineStopped = true;
            break;
        }

        if (bMineStopped) {
            console.error("My Broadcast is stopped!!!!!");
            this._onBroadcastDone(false);
            this._showAlertModal("방송 종료", "관리자에의해 방송이 강제로 종료되었습니다.");
        }
    }

    _showAlertModal(szTitle, szAlert) {
        //width: 244px; height: 394px;
        let html = `
            <div id="modal-sounds" class="i-modal">
                <div style="display: flex; margin: 15% auto; width: 488px;">
                    <div class="i-modal-list">
                        <div class="i-modal-list-title">
                            <div class="i-modal-list-title-main">${szTitle}</div>
                        </div>
                        <div style="background-color: var(--penta-box-2nd); border-radius: var(--penta-base-radius);">
                            <center><br/>${szAlert}<br/></center> <br/>
                        </div>
                        <div id="hide_alert_modal" class="i-modal-list-btn-close">확인</div>
                    </div>
                </div>
            </div>
        `;

        gDOM(DIVID_MAINCTS_MODAL).innerHTML = html;
        gDOM('hide_alert_modal').addEventListener("click", this._hideModalLayout.bind(this));
    }    

    _finalizeBroadcast(bResetOutput) {
        let dom = gDOM("bdc_startstop");
        dom.innerText = "방송 대기";
        this.broadcast.state = BRDSTATE_IDLE;
        if (this.broadcast.gpio_signals.length > 0) {
            for(let g of this.broadcast.gpio_signals) {
                if (g.pin == GPIOSIGNAL_MEDIA) g.value = 1;
                else g.value = 0;            
            }
            let jsGpio = { act: 'SET_GPIO_VALUES', payload: {pins: this.broadcast.gpio_signals} };        
            //console.log("GPIO4: " + JSON.stringify(jsGpio.payload));
            this.funcCallNative(JSON.stringify(jsGpio));
            this.broadcast.gpio_signals = [];
        }
        //방송이 종료되면 LRX가 다 꺼지기 때문에 출력 선택을 자동으로 해제한다
        if (this.selectedMtx != null) {
            if (bResetOutput) {
                for(let eg of this.broadcast.groups) {
                    this.selectedMtx.onGroupSelect(eg, false);
                }
                this.broadcast.groups = [];
                this._refreshMTX(this.selectedMtx);
            }
            else {
                //방송종료되면 LRX가 자동으로 꺼지기 때문에 강제로 한번 더 켠다
                let oldGrps = [...this.broadcast.groups];
                setTimeout(this._reSelectGroups.bind(this, oldGrps), 500); //0.5초 후에 그룹 선택 강제하기
            }
        }
    }

    _selectMIC(mtxConn) {
        let myTX = mtxConn.getMyTXDev();

        if (this.broadcast.txch === myTX.tx_channels[0].idx) {
            gDOM("select-mic").classList.remove("custom-click");
            this.broadcast.txch = 0;            
        }
        else {
            this._disableMediaSelection();
            if (this.broadcast.txch > 0) {
                let dom = gDOM("bdc_txch_" + this.broadcast.txch);
                if (dom !== null) dom.classList.remove("custom-click");
            }

            gDOM("select-mic").classList.add("custom-click");
            this.broadcast.txch = myTX.tx_channels[0].idx;
        }
        this._onSelectionChange();
    }

    _disableMediaSelection() {
        this.broadcast.medias = [];    
        let dom = gDOM("show-sound-modal-btn");
        dom.classList.remove("custom-click");
        dom.innerText = "음원";

        //로컬 음원 모두 해제
        for (let m of this.localMediaDOM_IDs) {            
            dom = gDOM(m);
            if (dom !== null) dom.classList.remove("custom-click");
        }
    }

    _onBroadCastStartFailed(szErr) {     
        let szMsg = "오류코드: " + szErr;
        if (szErr == "ERROR CASTORDER HIGHER EXIST") szMsg = "선택한 채널에 더 높은 우선순위의 방송이 이미 진행 중입니다.";
        this._showAlertModal("방송제어 실패", "방송 시작 요청이 실패하였습니다.<br/>" + szMsg);
        this._finalizeBroadcast(false);
    }

    //개별 앰프 제어
    _controlSpeakers(bIsEnable, spkIdxList) {
        let preDev = null;
        let ctrlList = [];
        let tch = null;
        let spk = null;

        if (this.selectedMtx == null) return;
        
        for(let spkIdx of spkIdxList) {
        let tdev = this.selectedMtx.getDeviceBySpeakerIdx(spkIdx);          
        if (tdev == null) continue;    
        tch = null;
        spk = null;
        for (let ec of tdev.rx_channels) {
            for (let es of ec.speakers) {
            if (es.idx == spkIdx) {
                tch = ec;
                spk = es;
                break;
            }
            }
            if (tch != null) break;        
        }  
        if (tch == null || spk == null) {
            console.error("Invalid speaker.. " + spkIdx);
            return;
        }

        //console.log(`SpkIdx=${spkIdx}, ch=${tch.order}, code1=${spk.code1}, code2=${spk.code2}`);
        if (tdev != preDev) {
            if (preDev != null && ctrlList.length > 0) {
            this.controlESPK_List.push({enable: bIsEnable, dev_name: preDev.name, speakers: ctrlList});
            }
            preDev = tdev;
            ctrlList = [];
        }
        ctrlList.push({idx: spkIdx, ch_order: tch.order, amp_order: spk.order, code1: spk.code1, code2: spk.code2});
        }

        if (ctrlList.length > 0) {
            this.controlESPK_List.push({enable: bIsEnable, dev_name: preDev.name, speakers: ctrlList});
            showWaitModal(true);
            this._callControlSpeaker();
        }        
    }

    _callControlSpeaker() {    
        if (this.controlESPK_List.length > 0) {
            //console.log("Call control speaker!!!!");
            let dat = this.controlESPK_List[0];      
            let payload = { job: dat };    
            this.controlESPK_List.splice(0, 1);      
            this.selectedMtx.rest_call(
                "control_each_speaker",
                payload,
                null,
                function (bRes, jsRecv, callParam) {
                    if (bRes != true || jsRecv.res != true) {
                        showWaitModal(false);
                        this._showAlertModal("오류", "스피커 제어 실패.");
                        return;
                    }
                    if (this.controlESPK_List.length > 0) {
                        this._callControlSpeaker();                
                    }
                    else showWaitModal(false);
                }.bind(this)
            );    

            return true;
        }
        return false;
    }  

    _makeVolumeLayout(szName, idN, nSize) {        
        let o8px = parseInt(8 * nSize / 200);
        let o12px = parseInt(12 * nSize / 200);
        let o14px = parseInt(14 * nSize / 200);
        let o18px = parseInt(18 * nSize / 200);
        let o20px = parseInt(20 * nSize / 200);
        let o32px = parseInt(32 * nSize / 200);
        let o50px = parseInt(50 * nSize / 200);
        let o60px = parseInt(60 * nSize / 200);
        let o100px = parseInt(100 * nSize / 200);

        let szLHBar = "";
        for(let i = 0; i < 11; i++) {
            szLHBar += `
                <div class="border-b-4 border-white"></div>
                <div class="h-full"></div>
            `;
        }
        szLHBar += `<div class="border-b-4 border-white"></div>`;

        let res = `
        <div class="w-[${nSize}px] h-full mx-[${o8px}px]">
            <div class="flex justify-center items-center h-full bg-[#232326] rounded-[${o8px}px] px-[${o20px}px] pt-[${o20px}px] pb-[${o50px}px]">
                <div class="h-full w-[${o32px}px] mr-[${o12px}px]">
                    <div class="h-[${o32px}px]">
                        <div class="truncate w-[${o100px}px] text-[${o18px}px]">${szName}</div>
                    </div>
                    <div class="flex flex-col justify-between h-full">
                        ${szLHBar}
                    </div>
                </div>
                <div class="h-full w-[${o60px}px]">
                    <div class="h-[${o32px}px]"></div>
                    <div class="h-full">
                        <div class="w-full h-full" id="main_vol_slide_${idN}"></div>
                    </div>
                </div>
                <div class="h-full w-[${o32px}px] ml-[${o12px}px]">
                    <div id="vm_attr_cur_${idN}" class="h-[${o32px}px]"></div>
                    <div class="flex flex-col justify-between h-full">
                        <div class="border-b-4 border-white"></div>
                        <div class="flex justify-end items-end text-[${o14px}px] h-full">90</div>
                        <div class="border-b-4 border-white"></div>
                        <div class="h-full">
                            <div class="flex justify-between h-full">
                                <div class="flex-shrink-0 w-4/12">
                                    <div class="border-b border-gray-400 h-1/5"></div>
                                    <div class="border-b border-gray-400 h-1/5"></div>
                                    <div class="border-b border-gray-400 h-1/5"></div>
                                    <div class="border-b border-gray-400 h-1/5"></div>
                                    <div class="h-1/5"></div>
                                </div>
                                <div class="flex items-end w-6/12 text-[${o14px}px]">80</div>
                            </div>
                        </div>
                        <div class="border-b-4 border-white"></div>
                        <div class="h-full">
                            <div class="flex justify-between h-full">
                                <div class="flex-shrink-0 w-4/12">
                                    <div class="border-b border-gray-400 h-1/5"></div>
                                    <div class="border-b border-gray-400 h-1/5"></div>
                                    <div class="border-b border-gray-400 h-1/5"></div>
                                    <div class="border-b border-gray-400 h-1/5"></div>
                                    <div class="h-1/5"></div>
                                </div>
                                <div class="flex items-end w-6/12 text-[${o14px}px]">70</div>
                            </div>
                        </div>
                        <div class="border-b-4 border-white"></div>
                        <div class="flex justify-end items-end text-[${o14px}px] h-full">60</div>
                        <div class="border-b-4 border-white"></div>
                        <div class="flex justify-end items-end text-[${o14px}px] h-full">50</div>
                        <div class="border-b-4 border-white"></div>
                        <div class="flex justify-end items-end text-[${o14px}px] h-full">40</div>
                        <div class="border-b-4 border-white"></div>
                        <div class="flex justify-end items-end text-[${o14px}px] h-full">30</div>
                        <div class="border-b-4 border-white"></div>
                        <div class="flex justify-end items-end text-[${o14px}px] h-full">20</div>
                        <div class="border-b-4 border-white"></div>
                        <div class="flex justify-end items-end text-[${o14px}px] h-full">10</div>
                        <div class="border-b-4 border-white"></div>
                        <div class="flex justify-end items-end text-[${o14px}px] h-full">0</div>
                        <div class="border-b-4 border-white"></div>
                    </div>
                </div>
            </div>
        </div>
        `;

        return res;
    }

    _onVolumeSet(idx) {
        let cVolume = document.querySelector(`#main_vol_slide_${idx}_sld_btn > div`).innerText;        
        if (idx == 0) {
            this._setMainboardVolume(parseInt(cVolume));
        }
        else {
            let vol = [];            
            vol.push({ channel: idx - 1, volume: mainCHVolConversion(false, cVolume) });
            console.log("Set Volume:\n" + JSON.stringify(vol));

            let reqV = { act: "SET_HARDWARE_INFO", payload: { type: "adc_volume", value: vol } };        
            this.funcCallNative(JSON.stringify(reqV));
        }        
    }

    _getHardwareInfo() {
        let reqV = { act: "GET_HARDWARE_INFO", payload: "adc_volume" };
        this.funcCallNative(JSON.stringify(reqV));
    }

    _getMainboardVolume() {
        let cmdPayload = { cmd: "amixer get Master", cmd_id: 100, need_result: true };
        let jsv = { act: "SYS_RUN_COMMAND", payload: cmdPayload };
        this.funcCallNative(JSON.stringify(jsv));
    }

    _setMainboardVolume(iVol) {
        let cmdPayload = { cmd: `amixer set Master ${iVol}%`, cmd_id: 101, need_result: false };
        let jsv = { act: "SYS_RUN_COMMAND", payload: cmdPayload };
        this.funcCallNative(JSON.stringify(jsv));
    }
    
    _parseLinuxVolume(szRes) {
        let szPer = parseLinuxVolume(szRes);
        let dom = document.querySelector(`#main_vol_slide_0_sld_btn > div`);
        if (dom == null) {
            console.log("ERROR: Channel for volume is not exist");
            return;
        }
        dom.innerText = szPer;        
        this.volumeSliders[0].SetValue(parseInt(szPer) / 100);
    }

    _setChannelVolume(volumes) {
        //volumes = [{channel: 0, volume: 0}, ... ]
        let dom = null;
        let tCh = 0;
        let tVS = this.volumeSliders[0];
        if (this.volumeSlideType < 1) return;

        if (this.volumeSlideType >= 10) {
            if(this.volumeSliders.length < 2) return;
            tCh = 0;
            tVS = this.volumeSliders[1];            
            dom = document.querySelector(`#main_vol_slide_1_sld_btn > div`);
        }
        else {
            tCh = this.volumeSlideType - 1;
            tVS = this.volumeSliders[0];            
            dom = document.querySelector(`#main_vol_slide_${this.volumeSlideType}_sld_btn > div`);
        }

        if(dom == null) return;
        for (let vol of volumes) {
            if (vol.channel != tCh) continue;
            dom.innerText = parseInt(vol.volume * 100).toString();
            tVS.SetValue(vol.volume);
        }
    }

    _onGridScroll() {
        this.grid_scroll_pos = gDOM('grp_grid_main').scrollTop;        
    }

    _onInputScroll() {
        this.input_scroll_pos = gDOM('input_list_main').scrollTop;        
    }

    _onSelectionChange() {        
        if (this.selectedMtx == null) return;

        let bCastOK = true;

        if (this.broadcast.medias.length < 1 && this.broadcast.txch == 0) bCastOK = false;        
        if (this.broadcast.groups.length < 1) bCastOK = false;

        if (this.preReadyState == bCastOK) return;
        this.preReadyState = bCastOK;

        let dom = gDOM("bdc_startstop");
        let doms = gDOM("bdc_chime_st");        
        if (bCastOK) {            
            dom.innerText = "방송 시작";
            dom.classList.add("custom-click");
            if (this.broadcast.autoChime != true) doms.classList.add("custom-click");
        }
        else {
            dom.innerText = "방송 대기";
            dom.classList.remove("custom-click");
            doms.classList.remove("custom-click");
        }
    }

    _reSelectGroups(grps) {
        console.log("Reset output called!!!");
        console.log(JSON.stringify(grps));
        if (this.selectedMtx == null) return;
        let spkList = [];
        for (let g of grps) {
            let tid = "bdc_group_list_" + g;          
            let tSpks = null;
            if(this.broadcast.groups.indexOf(g) < 0) tSpks = this._onGroupSelect(tid, g, true);
            if (tSpks != null) spkList = [...spkList, ...tSpks];
        }
        if (spkList.length > 0) this._controlSpeakers(true, spkList);        
    }

};

export { BroadcastMain };
