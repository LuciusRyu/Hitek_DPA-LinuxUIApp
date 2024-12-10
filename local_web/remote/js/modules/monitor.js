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
        let szLst = `<ul class="flex flex-row h-full">`;
        this.mtxList = mtxList;
        let iLast = -1;
        let i;
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
                    <div class="flex justify-center items-center h-[28px] mt-[16px] text-[22px]"></div>
                    <div class="mt-[16px] mb-[48px]">
                        <div class="flex flex-col justify-center items-center h-[60px] bg-[#343437] rounded-[8px]">
                            <div class="text-[14px]">STATUS</div>
                            <div class="text-[14px]">&nbsp;</div>
                        </div>
                        <div class="flex h-[60px] mt-[8px]">
                            <div class="flex flex-col justify-center items-center flex-1 bg-[#343437] rounded-[8px]">
                                <div class="text-[14px]">HFP</div>
                                <div class="text-[14px]">&nbsp;</div>
                            </div>
                            <div class="flex flex-col justify-center items-center flex-1 bg-[#343437] rounded-[8px] ml-[8px]">
                                <div class="text-[14px]">LIMIT</div>
                                <div class="text-[14px]">&nbsp;</div>
                            </div>
                        </div>
                        <div class="flex h-[60px] mt-[8px]">
                            <div class="flex flex-col justify-center items-center flex-1 bg-[#343437] rounded-[8px]">
                                <div class="text-[14px]">POWER</div>
                                <div class="text-[14px]">&nbsp;</div>
                            </div>
                            <div class="flex flex-col justify-center items-center flex-1 bg-[#343437] rounded-[8px] ml-[8px]">
                                <div class="text-[14px]">IMP</div>
                                <div class="text-[14px]">&nbsp;</div>
                            </div>
                        </div>
                        <div class="flex h-[60px] mt-[8px]">
                            <div class="flex flex-col justify-center items-center flex-1 bg-[#343437] rounded-[8px]">
                                <div class="text-[14px]">EM-VOL</div>
                                <div class="text-[14px]">&nbsp;</div>
                            </div>
                            <div class="flex flex-col justify-center items-center flex-1 bg-[#343437] rounded-[8px] ml-[8px]">
                                <div class="text-[14px]">TEMP</div>
                                <div class="text-[14px]">&nbsp;</div>
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
                            <div class="relative w-full h-full">
                                <div
                                    id="volume_level_range"
                                    class="absolute top-0 w-[8px] h-full transform -translate-x-1/2 left-1/2 border-0 rounded-[22px] bg-white"
                                ></div>
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

        for (let dev of mtxConn.devList) {
            if (!dev.name.startsWith(getDevModelPrefix("PA"))) continue;

            let devNick = dev.nick_name;
            if (devNick == "NONE") devNick = dev.name;

            ctid = "collapseTitle_" + i;
            ccid = "collapseContent_" + i;

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
                        style="display: none"
                    >
                        <div class="grid grid-cols-auto-156px gap-[12px]">
            `;

            epf = { id: ctid, fn: this._onAmpCollapseClicked.bind(this, ctid, ccid) };
            evtPairList.push(epf);

            for (let rxc of dev.rx_channels) {
                let tid = "mm_amp_" + dev.idx + "_" + rxc.idx;
                epf = { id: tid, fn: this._onAmpClicked.bind(this, tid, mtxConn, dev, rxc) };                
                let szClass = "p-[12px] bg-[#343437] rounded-[8px] cursor-pointer";
                if (mtxConn.checkChannelInMonitoring(rxc.idx)) szClass += " border-[2px] border-yellow-300";
                else evtPairList.push(epf);

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

            i++;
        }

        this.selectedAmp = null;

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

        let html;
        if (bEnable) {
            if (tdev.nick_name == "NONE")
                html = `<div class="flex justify-center items-center h-[28px] mt-[16px] text-[22px]">${tdev.name} : ${rxc.nick_name}</div>`;
            else
                html = `<div class="flex justify-center items-center h-[28px] mt-[16px] text-[22px]">${tdev.nick_name} : ${rxc.nick_name}</div>`;
        } else html = `<div class="flex justify-center items-center h-[28px] mt-[16px] text-[22px]">선택없음</div>`;
        html += `
            <div class="mt-[16px] mb-[48px]">
                <div class="flex flex-col justify-center items-center h-[60px] bg-[#343437] rounded-[8px]">
                    <div class="text-[14px]">STATUS</div>
                    <div class="text-[14px]">&nbsp;</div>
                </div>
                <div class="flex h-[60px] mt-[8px]">
                    <div class="flex flex-col justify-center items-center flex-1 bg-[#343437] rounded-[8px]">
                        <div class="text-[14px]">HFP</div>
                        <div class="text-[14px]">&nbsp;</div>
                    </div>
                    <div class="flex flex-col justify-center items-center flex-1 bg-[#343437] rounded-[8px] ml-[8px]">
                        <div class="text-[14px]">LIMIT</div>
                        <div class="text-[14px]">&nbsp;</div>
                    </div>
                </div>
                <div class="flex h-[60px] mt-[8px]">
                    <div class="flex flex-col justify-center items-center flex-1 bg-[#343437] rounded-[8px]">
                        <div class="text-[14px]">POWER</div>
                        <div class="text-[14px]">&nbsp;</div>
                    </div>
                    <div class="flex flex-col justify-center items-center flex-1 bg-[#343437] rounded-[8px] ml-[8px]">
                        <div class="text-[14px]">IMP</div>
                        <div class="text-[14px]">&nbsp;</div>
                    </div>
                </div>
                <div class="flex h-[60px] mt-[8px]">
                    <div class="flex flex-col justify-center items-center flex-1 bg-[#343437] rounded-[8px]">
                        <div class="text-[14px]">EM-VOL</div>
                        <div class="text-[14px]">&nbsp;</div>
                    </div>
                    <div class="flex flex-col justify-center items-center flex-1 bg-[#343437] rounded-[8px] ml-[8px]">
                        <div class="text-[14px]">TEMP</div>
                        <div class="text-[14px]">&nbsp;</div>
                    </div>
                </div>
            </div>
        `;

        dom.innerHTML = html;
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
            //시작
            this._controlUISelection(true, szId, tdev, rxc);
            this.selectedAmp = { szID: szId, mtxConn: mtxConn, dev: tdev, rxc: rxc, seq: 0 };
            this._rest_startStop_mornitor(true, this.selectedAmp);
        }
    }

    _onVolumeChange() {
        let dom = gDOM("mm_selected_amp_vol");
        if (dom == null || this.selectedAmp == null) return;
        this._rest_changeVolume(this.selectedAmp, parseInt(dom.value));
    }

    _rest_changeVolume(objAmp, vol) {
        let payload = { ch_idx: objAmp.rxc.idx, volume: vol };
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
        let myTX = this.connector.getMyTXDev();
        if (myTX == null) return;

        if (bStart != true) {
            if (this.timer != null) clearInterval(this.timer);
            this.timer = null;
        }

        let payload = { start: bStart, ch_idx: objAmp.rxc.idx, rx_dev: myTX.idx };
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
                                alert("채널 모니터링 시작 실패");
                            }
                        } else {
                            alert("채널 모니터링 중지 실패");
                        }
                        return;
                    }
                    if (callParam == true) {
                        console.log("Start monitoring timer");
                        if (this.timer != null) clearInterval(this.timer);
                        this.timer = setInterval(this._channelRefreshLoop.bind(this), 200);
                    }
                } else {
                    alert("채널 모니터링 제어 실패");
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

    _refreshAmpState(jsRes) {
        if (jsRes.b64_val == "NONE") {
            console.log("Warning - No status value..");
            return;
        }
        let values = Base64.decode(jsRes.b64_val);
        let res = JSON.parse(values);
        let dom = gDOM("mm_selected_amp_state");

        let devNick = this.selectedAmp.dev.nick_name;
        if (devNick == "NONE") devNick = this.selectedAmp.dev.name;
        let html = `
            <div class="flex justify-center items-center h-[28px] mt-[16px] text-[22px]">${devNick} : ${this.selectedAmp.rxc.nick_name}</div>
                <div class="mt-[16px] mb-[48px]">
                <div class="flex flex-col justify-center items-center h-[60px] bg-[#343437] rounded-[8px]">
                    <div class="text-[14px]">STATUS</div>
                    <div class="text-[14px]">${this.selectedAmp.rxc.state.status}</div>
                </div>
                <div class="flex h-[60px] mt-[8px]">
                    <div class="flex flex-col justify-center items-center flex-1 bg-[#343437] rounded-[8px]">
                        <div class="text-[14px]">HFP</div>
                        <div class="text-[14px]">${res.hfp}</div>
                    </div>
                    <div class="flex flex-col justify-center items-center flex-1 bg-[#343437] rounded-[8px] ml-[8px]">
                        <div class="text-[14px]">LIMIT</div>
                        <div class="text-[14px]">${res.limit}</div>
                    </div>
                </div>
                <div class="flex h-[60px] mt-[8px]">
                    <div class="flex flex-col justify-center items-center flex-1 bg-[#343437] rounded-[8px]">
                        <div class="text-[14px]">POWER</div>
                        <div class="text-[14px]">${res.power}</div>
                    </div>
                    <div class="flex flex-col justify-center items-center flex-1 bg-[#343437] rounded-[8px] ml-[8px]">
                        <div class="text-[14px]">IMP</div>
                        <div class="text-[14px]">${res.imp}</div>
                    </div>
                </div>
                <div class="flex h-[60px] mt-[8px]">
                    <div class="flex flex-col justify-center items-center flex-1 bg-[#343437] rounded-[8px]">
                        <div class="text-[14px]">EM-VOL</div>
                        <div class="text-[14px]">${res.em_vol}</div>
                    </div>
                    <div class="flex flex-col justify-center items-center flex-1 bg-[#343437] rounded-[8px] ml-[8px]">
                        <div class="text-[14px]">TEMP</div>
                        <div class="text-[14px]">${res.temp}</div>
                    </div>
                </div>
            </div>
        `;

        dom.innerHTML = html;

        // dom = gDOM("mm_selected_amp_level");
        // dom.innerText = "출력 레벨: " + res.level;
        //설정 볼륨값 = res.vol
    }

    _onAmpCollapseClicked(ctid, ccid) {
        const collapseTitle = document.getElementById(ctid);
        const collapseContent = document.getElementById(ccid);

        if (collapseContent.style.display == "none") {
            collapseContent.style.display = "block";
            collapseTitle.style.borderBottomLeftRadius = "0px";
            collapseTitle.style.borderBottomRightRadius = "0px";
        } else {
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
            // this.sliders[i].SetOnChangeCallback(this._onVolumeChange.bind(this));
        }
    }
};

export { MonitorMain };
