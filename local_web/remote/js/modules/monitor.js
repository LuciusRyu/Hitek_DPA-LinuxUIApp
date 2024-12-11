import { PentaMoniterSlide } from "./penta_moniter_slider.js";

const COLOR_MULTITAB_NOR = "#232326";
const COLOR_MULTITAB_SEL = "#343437";

const MonitorMain = class monitor_main {
    constructor(funcCallNative, connector) {
        this.funcCallNative = funcCallNative;
        this.connector = connector;
        this.mtxList = null;
        this.lastSelection = {uuid: "", dom_id: ""};
        this.selectedMtx = null;
        this.selectedAmp = null;
        this.timer = null;
        this.sliders = null;
        this.collapseStateList = []; //{mtxSeq: 0, devList: []}
    }

    BuildLayout() {
        this._refreshLayout(this.connector.getAvailConnections());
    }

    on_NativeCall(jsV) {}

    on_ConnectorEvent(szEvt, jsV) {
        if (szEvt == EVTSTR_CONN_CHANGED) this._refreshLayout(jsV);
        else if(szEvt == EVTSTR_DATA_CHANGED) {
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

            if (this.selectedMtx == null || this.selectedMtx.mtxInfo.uuid != jsV.mtxConn.mtxInfo.uuid) return;
            for (let i = 0; i < this.mtxList.length; i++) {                                
                if(this.mtxList[i].mtxInfo.uuid == this.selectedMtx.mtxInfo.uuid) {
                    this._selectMTX(i, true);
                    break;
                }
            }            
        }
    }

    onDisabled() {
        if (this.selectedAmp != null) {
            this._rest_startStop_mornitor(false, this.selectedAmp);
            this.selectedAmp = null;
        }
    }

    _refreshLayout(mtxList) {
        let i;
        //리스트 정리 - 없어진 애들 삭제
        let bContinue = true;
        while(bContinue) {
            bContinue = false;
            for(i = 0; i < this.collapseStateList.length; i++) {
                let bExist = false;
                for(let em of mtxList) {
                    if (em.uniq_seq == this.collapseStateList[i].mtxSeq) {
                        bExist = true;
                        break;
                    }
                }
                if (!bExist) {
                    this.collapseStateList.splice(i, 1);
                    bContinue = true;
                    break;
                }
            }
        }
        //리스트 정리 - 신규 추가
        for(let em of mtxList) {
            let bExist = false;
            for(let ec of this.collapseStateList) {
                if (ec.mtxSeq == em.uniq_seq) {
                    bExist = true;
                    break;
                }
            }
            if (!bExist) this.collapseStateList.push({mtxSeq: em.uniq_seq, devList: []});
        }

        let szLst = `<ul class="flex flex-row h-full">`;
        this.mtxList = mtxList;
        let iLast = -1;
        
        if (mtxList.length < 1) {
            szLst += `
                <li class="flex justify-center items-center h-full cursor-pointer px-[20px] first:bg-[#343437] first:rounded-tl-lg last:rounded-tr-lg">
                    서버없음
                </li>
            `;
            this.lastSelection = {uuid: "", dom_id: ""};
        } else {
            //마지막 선택했던 서버가 아직 있나?
            for (i = 0; i < mtxList.length; i++) {
                if (mtxList[i].mtxInfo.uuid == this.lastSelection.uuid) {
                    iLast = i;
                    break;
                }
            }
            if (iLast < 0) iLast = 0;
            for (i = 0; i < mtxList.length; i++) {
                let mtxNick = mtxList[i].mtxInfo.id;
                let mtxD = mtxList[i].getMainTXDev();
                if (mtxD != null) {
                    if (mtxD.nick_name != "NONE") mtxNick = mtxD.nick_name;
                }                                 

                szLst += `<li class="flex justify-center items-center h-full cursor-pointer px-[20px] first:bg-[${COLOR_MULTITAB_NOR}] first:rounded-tl-lg last:rounded-tr-lg"`;
                szLst += `' id='mm_mtxlist_${i}'>${mtxNick}</li>`;
            }
        }
        szLst += "</ul>";

        let html = `
            <div class="flex w-full h-full">
                <aside id="mm_cts_side" class="flex-none h-full w-[208px]"></aside>
                <section class="h-full grow ml-[12px]">
                    <div class="items-center h-[60px] bg-[${COLOR_MULTITAB_NOR}] rounded-t-lg flex">
                        ${szLst}
                    </div>
                    <div class="w-full h-[calc(100%-60px)] bg-[${COLOR_MULTITAB_SEL}] rounded-b-lg">
                        <div class="flex flex-row w-full h-full p-[8px]">
                            <section id="mm_cts_main" class="flex-auto w-full h-full overflow-auto custom-scrollbar"></section>
                        </div>
                    </div>
                </section>
            </div>
        `;

        gDOM(DIVID_MAINCTS).innerHTML = html;

        if (iLast >= 0) {
            for (i = 0; i < mtxList.length; i++) {
                gDOM("mm_mtxlist_" + i).addEventListener("click", this._selectMTX.bind(this, i, false));
            }
            this._selectMTX(iLast, true);
        }
    }

    _selectMTX(idx, bForceRefresh) {
        if (idx < 0 || idx >= this.mtxList.length) return;
        let tmtx = this.mtxList[idx];                
        if (!bForceRefresh && this.lastSelection.uuid == tmtx.mtxInfo.uuid) return;

        let dom = gDOM(this.lastSelection.dom_id);
        if (dom != null) dom.style.backgroundColor = COLOR_MULTITAB_NOR;

        this.lastSelection.uuid = tmtx.mtxInfo.uuid;
        this.lastSelection.dom_id = "mm_mtxlist_" + idx;
        gDOM(this.lastSelection.dom_id).style.backgroundColor = COLOR_MULTITAB_SEL;

        this._refreshMTX(tmtx);
    }

    _refreshMTX(mtxConn) {
        this.selectedMtx = mtxConn;

        let evtPairList = [];
        let epf;
    
        let szState = `
            <div class="flex flex-col h-full px-[12px] bg-[#232326] rounded-[8px]">
                <div id="mm_selected_amp_state">
                    <div id="amp_in_status_title" class="flex justify-center items-center h-[28px] mt-[16px] text-[22px]">선택없음</div>
                    <div class="mt-[16px] mb-[48px]">
                        <div class="flex flex-col justify-center items-center h-[60px] bg-[#343437] rounded-[8px]">
                            <div class="text-[14px]">STATUS</div>
                            <div id="amp_in_status_status" class="text-[14px]">&nbsp;</div>
                        </div>
                        <div class="flex h-[60px] mt-[8px]">
                            <div class="flex flex-col justify-center items-center flex-1 bg-[#343437] rounded-[8px]">
                                <div class="text-[14px]">HFP</div>
                                <div id="amp_in_status_filter" class="text-[14px]">&nbsp;</div>
                            </div>
                            <div class="flex flex-col justify-center items-center flex-1 bg-[#343437] rounded-[8px] ml-[8px]">
                                <div class="text-[14px]">LIMIT</div>
                                <div id="amp_in_status_limit" class="text-[14px]">&nbsp;</div>
                            </div>
                        </div>
                        <div class="flex h-[60px] mt-[8px]">
                            <div class="flex flex-col justify-center items-center flex-1 bg-[#343437] rounded-[8px]">
                                <div class="text-[14px]">POWER</div>
                                <div id="amp_in_status_power" class="text-[14px]">&nbsp;</div>
                            </div>
                            <div class="flex flex-col justify-center items-center flex-1 bg-[#343437] rounded-[8px] ml-[8px]">
                                <div class="text-[14px]">IMP</div>
                                <div id="amp_in_status_imp" class="text-[14px]">&nbsp;</div>
                            </div>
                        </div>
                        <div class="flex h-[60px] mt-[8px]">
                            <div class="flex flex-col justify-center items-center flex-1 bg-[#343437] rounded-[8px]">
                                <div class="text-[14px]">EM-VOL</div>
                                <div id="amp_in_status_em_volm" class="text-[14px]">&nbsp;</div>
                            </div>
                            <div class="flex flex-col justify-center items-center flex-1 bg-[#343437] rounded-[8px] ml-[8px]">
                                <div class="text-[14px]">TEMP</div>
                                <div id="amp_in_status_temp" class="text-[14px]">&nbsp;</div>
                            </div>
                        </div>
                    </div>
                </div>
                <div class="flex items-center justify-center h-[296px] rounded-[8px] bg-[#343437] p-[10px]">
                    <div class="w-2/12 h-full">
                        <div class="h-[10%]">
                            <div class="flex justify-center items-end h-full text-[13px]">3db</div>
                        </div>
                        <div class="flex flex-col justify-between h-[90%]">
                            <div class="border border-white"></div>
                            <div class="flex justify-end items-end h-full text-[12px]">0</div>
                            <div class="border border-white"></div>
                            <div class="flex justify-end items-end h-full text-[12px]">-3</div>
                            <div class="border border-white"></div>
                            <div class="flex justify-end items-end h-full text-[12px]">-6</div>
                            <div class="border border-white"></div>
                            <div class="flex justify-end items-end h-full text-[12px]">-9</div>
                            <div class="border border-white"></div>
                            <div class="flex justify-end items-end h-full text-[12px]">-16</div>
                            <div class="border border-white"></div>
                            <div class="flex justify-end items-end h-full text-[12px]">-22</div>
                            <div class="border border-white"></div>
                            <div class="flex justify-end items-end h-full text-[12px]">-30</div>
                            <div class="border border-white"></div>
                            <div class="flex justify-end items-end h-full text-[12px]">-40</div>
                            <div class="border border-white"></div>
                        </div>
                    </div>

                    <div class="w-3/12 h-full">
                        <div class="flex justify-center items-center h-[10%]">
                            <div class="w-[20px] h-[10px] bg-white" id="peak_box"></div>
                        </div>
                        <div class="h-[90%]">
                            <div class="barcontainer">
                                <div class="bar" id="volume_level_range" style="height: 1%;"></div>
                            </div>                        
                        </div>
                    </div>

                    <div class="w-2/12 h-full">
                        <div class="h-[10%]">
                            <div class="flex justify-center items-end h-full text-[11px]">PEAK</div>
                        </div>
                        <div class="flex flex-col justify-between h-[90%]">
                            <div class="border border-white"></div>
                            <div class="flex justify-end items-end h-full text-[12px]"></div>
                            <div class="border border-white"></div>
                            <div class="h-full">
                                <div class="flex justify-between h-full">
                                    <div class="flex items-end w-4/12"></div>
                                    <div class="flex-shrink-0 w-6/12">
                                        <div class="border-b border-white h-1/4"></div>
                                        <div class="border-b border-white h-1/4"></div>
                                        <div class="border-b border-white h-1/4"></div>
                                        <div class="h-1/4"></div>
                                    </div>
                                </div>
                            </div>
                            <div class="border border-white"></div>
                            <div class="h-full">
                                <div class="flex justify-between h-full">
                                    <div class="flex items-end w-4/12"></div>
                                    <div class="flex-shrink-0 w-6/12">
                                        <div class="border-b border-white h-1/4"></div>
                                        <div class="border-b border-white h-1/4"></div>
                                        <div class="border-b border-white h-1/4"></div>
                                        <div class="h-1/4"></div>
                                    </div>
                                </div>
                            </div>
                            <div class="border border-white"></div>
                            <div class="flex justify-end items-end h-full text-[12px]"></div>
                            <div class="border border-white"></div>
                            <div class="flex justify-end items-end h-full text-[12px]"></div>
                            <div class="border border-white"></div>
                            <div class="flex justify-end items-end h-full text-[12px]"></div>
                            <div class="border border-white"></div>
                            <div class="flex justify-end items-end h-full text-[12px]"></div>
                            <div class="border border-white"></div>
                            <div class="flex justify-end items-end h-full text-[12px]"></div>
                            <div class="border border-white"></div>
                            <div class="flex justify-end items-end h-full text-[12px]"></div>
                            <div class="border border-white"></div>
                            <div class="flex justify-end items-end h-full text-[12px]"></div>
                            <div class="border border-white"></div>
                        </div>
                    </div>

                    <div class="w-3/12 h-full">
                        <div class="h-[10%]">
                            <div style="width: 100%; height: 100%"></div>
                        </div>
                        <div class="h-[90%]">
                            <div class="w-full h-full" id="m_slide_0"></div>
                        </div>
                    </div>

                    <div class="w-2/12 h-full">
                        <div class="h-[10%]">
                            <div class="flex justify-end items-end h-full text-[11px]">100</div>
                        </div>
                        <div class="flex flex-col justify-between h-[90%]">
                            <div class="border border-white"></div>
                            <div class="flex justify-end items-end h-full text-[11px]">90</div>
                            <div class="border border-white"></div>
                            <div class="h-full">
                                <div class="flex justify-between h-full">
                                    <div class="flex-shrink-0 w-4/12">
                                        <div class="border-b border-gray-400 h-1/4"></div>
                                        <div class="border-b border-gray-400 h-1/4"></div>
                                        <div class="border-b border-gray-400 h-1/4"></div>
                                        <div class="h-1/4"></div>
                                    </div>
                                    <div class="flex items-end text-[11px] w-6/12">80</div>
                                </div>
                            </div>
                            <div class="border border-white"></div>
                            <div class="h-full">
                                <div class="flex justify-between h-full">
                                    <div class="flex-shrink-0 w-4/12">
                                        <div class="border-b border-gray-400 h-1/4"></div>
                                        <div class="border-b border-gray-400 h-1/4"></div>
                                        <div class="border-b border-gray-400 h-1/4"></div>
                                        <div class="h-1/4"></div>
                                    </div>
                                    <div class="flex items-end text-[11px] w-6/12">70</div>
                                </div>
                            </div>
                            <div class="border border-white"></div>
                            <div class="flex justify-end items-end h-full text-[11px]">60</div>
                            <div class="border border-white"></div>
                            <div class="flex justify-end items-end h-full text-[11px]">50</div>
                            <div class="border border-white"></div>
                            <div class="flex justify-end items-end h-full text-[11px]">40</div>
                            <div class="border border-white"></div>
                            <div class="flex justify-end items-end h-full text-[11px]">30</div>
                            <div class="border border-white"></div>
                            <div class="flex justify-end items-end h-full text-[11px]">20</div>
                            <div class="border border-white"></div>
                            <div class="flex justify-end items-end h-full text-[11px]">10</div>
                            <div class="border border-white"></div>
                            <div class="flex justify-end items-end h-full text-[11px]">0</div>
                            <div class="border border-white"></div>
                        </div>
                    </div>
                </div>
            </div>
        `;

        // epf = { id: "mm_selected_amp_vol_change", fn: this._onVolumeChange.bind(this) };
        // evtPairList.push(epf);

        document.getElementById("mm_cts_side").innerHTML = szState;

        let szAmps = ``;
        let i = 0;
        let ctid;
        let ccid;

        let collapseState = null;
        for(i = 0; i < this.collapseStateList.length; i++) {
            if (this.collapseStateList[i].mtxSeq == mtxConn.uniq_seq) {
                collapseState = this.collapseStateList[i];
                break;
            }
        }

        if (collapseState == null) {
            alert("Programming error!!! - _refreshMTX - NO list");
            return;
        }
        
        for (let dev of mtxConn.devList) {
            if (!dev.name.startsWith(getDevModelPrefix("PA"))) continue;

            //저장된 펼침 상태 가져오기
            let colState = null;       
            for(let es of collapseState.devList) {
                if (es.idx == dev.idx) {
                    colState = es;
                    break;
                }
            }
            if (colState == null) {
                colState = {idx: dev.idx, state: "none"};
                collapseState.devList.push(colState);
            }

            let devNick = dev.nick_name;
            if (devNick == "NONE") devNick = dev.name;

            ctid = "collapseTitle_" + dev.idx;
            ccid = "collapseContent_" + dev.idx;

            szAmps += `
                <div class="mb-[10px]">
                    <div
                        id="${ctid}"
                        class="flex items-center justify-between w-full cursor-pointer bg-[#232326] rounded-[8px] px-[12px] py-[10px]"
                    >
                        <h2 class="text-lg font-semibold">${devNick}</h2>
                        <button
                            class="text-gray-500 transition duration-150 ease-in-out focus:outline-none hover:text-gray-600"
                            style="transform: rotate(180deg)"
                        >
                            <svg class="w-5 h-5 fill-current" viewBox="0 0 20 20" fill="currentColor">
                                <path
                                    fill-rule="evenodd"
                                    d="M9.293 4.293a1 1 0 011.414 0l6 6a1 1 0 01-1.414 1.414L10 7.414l-5.293 5.293a1 1 0 01-1.414-1.414l6-6z"
                                    clip-rule="evenodd"
                                />
                            </svg>
                        </button>
                    </div>
                    <div
                        id="${ccid}"
                        class="w-full px-[12px] py-[10px] overflow-hidden bg-[#232326] rounded-b-[8px]"
                        style="display: ${colState.state}"
                    >
                        <div class="grid grid-cols-auto-156px gap-[12px]">
            `;

            epf = { id: ctid, fn: this._onAmpCollapseClicked.bind(this, dev.idx, collapseState) };
            evtPairList.push(epf);

            for (let rxc of dev.rx_channels) {
                let tid = "mm_amp_" + dev.idx + "_" + rxc.idx;
                epf = { id: tid, fn: this._onAmpClicked.bind(this, tid, mtxConn, dev, rxc) };                
                let szClass = "p-[12px] bg-[#343437] rounded-[8px] cursor-pointer";
                if (mtxConn.checkChannelInMonitoring(rxc.idx)) szClass += " border-[2px] border-yellow-300";
                else evtPairList.push(epf);

                if (this.selectedAmp != null) {
                    if (this.selectedAmp.mtxConn.uniq_seq == mtxConn.uniq_seq) {
                        if (this.selectedAmp.rxc.idx == rxc.idx) szClass += " custom-click";
                    }                    
                }
                szAmps += `
                    <div id="${tid}" class="${szClass}">
                        <div class="flex items-center justify-center text-center text-[14px]">${rxc.nick_name}</div>
                    </div>
                `;
            }

            szAmps += `
                        </div>
                    </div>
                </div>
            `;
        }        

        document.getElementById("mm_cts_main").innerHTML = szAmps;

        for (let epf of evtPairList) {
            gDOM(epf.id).addEventListener("click", epf.fn);
        }

        // 지연을 걸지않으면 dom을 읽어오기전에 슬라이더를 그려버려서 에러발생
        setTimeout(() => {
            this._setSlider();
        }, 500);
    }

    _controlUISelection(bEnable, szId, tdev, rxc) {
        let dom = gDOM(szId);
        if (bEnable) {
            dom.classList.add("custom-click");
        } else {
            dom.classList.remove("custom-click");
        }
        dom = gDOM("mm_selected_amp_state");

        let szTitle = "선택없음";
        if (bEnable) szTitle = rxc.nick_name;

        gDOM('amp_in_status_title').innerText = szTitle;
        gDOM('amp_in_status_status').innerText = " ";
        gDOM('amp_in_status_filter').innerText = " ";
        gDOM('amp_in_status_limit').innerText = " ";
        gDOM('amp_in_status_power').innerText = " ";
        gDOM('amp_in_status_temp').innerText = " ";
        gDOM('amp_in_status_imp').innerText = " ";        
        gDOM('amp_in_status_em_volm').innerText = " ";

        if (this.sliders != null && this.sliders.length > 0) this.sliders[0].SetValue(0);
        this._setVolumeMeter(0, false);
    }

    _onAmpClicked(szId, mtxConn, tdev, rxc) {
        if (this.selectedAmp != null && this.selectedAmp.szID == szId) {
            //중지
            this._controlUISelection(false, this.selectedAmp.szID, tdev, rxc);
            this._rest_startStop_mornitor(false, this.selectedAmp);
            this.selectedAmp = null;
        } else {
            //변경
            if (this.selectedAmp != null) {
                //중지
                this._controlUISelection(false, this.selectedAmp.szID, tdev, rxc);
                this._rest_startStop_mornitor(false, this.selectedAmp);
            }
            let szDup = mtxConn.checkAMPMonitoring(rxc.idx);
            if (szDup != null) {
                this._showAlertModal("오류", szDup);                
                return;
            }
            //시작
            this._controlUISelection(true, szId, tdev, rxc);
            this.selectedAmp = { szID: szId, mtxConn: mtxConn, dev: tdev, rxc: rxc, seq: 0 };
            this._rest_startStop_mornitor(true, this.selectedAmp);
        }
    }

    _onVolumeChange(szId, fVol) {        
        if (this.selectedAmp == null) return;
        //console.log("OnChange: " + fVol);
        this._rest_changeVolume(this.selectedAmp, parseInt(fVol * 100));
    }

    _rest_changeVolume(objAmp, vol) {
        let payload = { ch_idx: objAmp.rxc.idx, volume: vol };
        //console.log("Vol: " + JSON.stringify(payload));
        objAmp.mtxConn.rest_call(
            "set_channel_volume",
            payload,
            null,
            function (bRes, jsRecv, callParam) {
                if (bRes != true) {
                    alert("볼륨 제어 실패");
                    return;
                }
                if (jsRecv.res != true) {
                    alert("볼륨 제어 실패: " + jsRecv.error);
                    return;
                }
                console.log("Volume control success");
            }.bind(this)
        );
    }

    _rest_startStop_mornitor(bStart, objAmp) {
        if (objAmp.mtxConn == null) return;
        let myTX = objAmp.mtxConn.getMyTXDev();
        if (myTX == null) return;
        
        if (bStart != true) {
            if (this.timer != null) clearInterval(this.timer);
            this.timer = null;
        }

        let payload = { start: bStart, ch_idx: objAmp.rxc.idx, rx_dev: myTX.idx, userInfo: this.connector.userInfo };
        objAmp.mtxConn.rest_call(
            "channel_monitoring",
            payload,
            bStart,
            function (bRes, jsRecv, callParam) {
                if (bRes) {
                    if (jsRecv.payload == null) {
                        if (callParam == true) {
                            if (jsRecv.error_code == 5) {
                                //error: Target route exist already..
                                console.log("Error code = Existing route:" + jsRecv.error);
                            } else {
                                if (this.selectedAmp != null) {
                                    this._controlUISelection(false, this.selectedAmp.szID, null, null);
                                    this.selectedAmp = null;
                                }
                                console.log(JSON.stringify(jsRecv));                                
                            }
                            this._showAlertModal("오류", "채널 모니터링 시작 실패<br/>" + jsRecv.error);
                        } else {
                            this._showAlertModal("오류", "채널 모니터링 중지 실패");
                        }
                        return;
                    }
                    if (callParam == true) {
                        console.log("Start monitoring timer");
                        if (this.timer != null) clearInterval(this.timer);
                        this.timer = setInterval(this._channelRefreshLoop.bind(this), 200);
                    }
                } else {
                    this._showAlertModal("오류", "채널 모니터링 제어 실패");
                }
            }.bind(this)
        );
    }

    _channelRefreshLoop() {
        if (this.selectedAmp == null) return;
        this._rest_refreshChannel(this.selectedAmp);
    }

    _rest_refreshChannel(objAmp) {
        let payload = { ch_idx: objAmp.rxc.idx };
        objAmp.mtxConn.rest_call(
            "get_channel_status",
            payload,
            null,
            function (bRes, jsRecv, callParam) {
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
            }.bind(this)
        );
    }

    _setVolumeMeter(volLvl, bPeak) {
        if (volLvl < 1) volLvl = 1;
        let dom = gDOM('volume_level_range');

        if (dom != null) {
            dom.style.height = volLvl + '%';
        
            if (volLvl <= 73) dom.style.background = '#64FE2E';
            else if (volLvl <= 93) dom.style.background = '#F4FA58';
            else dom.style.background = '#FF0000';
        }

        dom = gDOM('peak_box');
        if (dom != null) {
            if (bPeak) dom.style.backgroundColor = 'red';
            else dom.style.backgroundColor = 'white';
        }
    }

    _refreshAmpState(jsRes) {
        if (jsRes.b64_val == "NONE") {
            console.log("Warning - No status value..");
            return;
        }
        let values = Base64.decode(jsRes.b64_val);
        let res = JSON.parse(values);

        gDOM('amp_in_status_status').innerText = this.selectedAmp.rxc.state.status;
        gDOM('amp_in_status_filter').innerText = res.hfp == 'O' ? 'On' : 'Off';
        gDOM('amp_in_status_limit').innerText = res.limit == 'O' ? 'On' : 'Off';
        gDOM('amp_in_status_power').innerText = res.power == 'A' ? 'AC' : 'DC';
        gDOM('amp_in_status_temp').innerText = res.temp + '℃';
        gDOM('amp_in_status_imp').innerText = res.imp == 'L' ? '70V' : '100V';        
        gDOM('amp_in_status_em_volm').innerText = res.em_vol;

        //console.log("Volume level = " + res.level);
        this._setVolumeMeter(parseInt(res.level), res.peak == 'O');
                
        if (this.sliders != null && this.sliders.length > 0) this.sliders[0].SetValue(parseInt(res.vol) / 100.0);
    }

    _onAmpCollapseClicked(devIdx, collapseState) {
        let ctid = "collapseTitle_" + devIdx;
        let ccid = "collapseContent_" + devIdx;
        let tState = null;
        for(let i = 0; i < collapseState.devList.length; i++) {
            if (collapseState.devList[i].idx == devIdx) {
                tState = collapseState.devList[i];
                break;
            }
        }

        const collapseTitle = document.getElementById(ctid);
        const collapseContent = document.getElementById(ccid);

        if (collapseContent.style.display == "none") {
            if (tState != null) tState.state = "block";
            collapseContent.style.display = "block";
            collapseTitle.style.borderBottomLeftRadius = "0px";
            collapseTitle.style.borderBottomRightRadius = "0px";
        } else {
            if (tState != null) tState.state = "none";
            collapseContent.style.display = "none";
            collapseTitle.style.borderBottomLeftRadius = "8px";
            collapseTitle.style.borderBottomRightRadius = "8px";
        }
    }

    _setSlider() {
        this.sliders = new Array();
        this.sliders.push(new PentaMoniterSlide("m_slide_0", 0, 1));

        for (let i = 0; i < this.sliders.length; i++) {
            this.sliders[i].Show();
             this.sliders[i].SetOnChangeCallback(this._onVolumeChange.bind(this));
        }
    }

    _showAlertModal(szTitle, szAlert) {
        //width: 244px; height: 394px;
        let html = `
            <div id="modal-alert" class="i-modal">
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

    _hideModalLayout(state) {
        gDOM("modal-alert").style.display = "none";
    }

};

export { MonitorMain };
