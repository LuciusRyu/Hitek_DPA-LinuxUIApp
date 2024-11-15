const remoteSetup = class remote_setup {
    constructor(funcCallNative) {
        this.page_name = 'Remote_Setup';
        this.funcCallNative = funcCallNative;
        this.sysInfo = null;
        this.curStep = 0;
        this.baseInfo = null;
        this.countdownInterval = null;
    }

    SetBaseInfo(baseInfo) {
        this.baseInfo = baseInfo;
    }

    BuildLayout() {
        let html = `
            <div class="flex items-center justify-center flex-col min-h-screen">
                <div class="mb-[20px]">
                    <img src="./assets/images/settingLogo.png" />
                    <div class="text-2xl text-center mt-[20px]">셋팅 설정 로딩중....</div>
                </div>
            </div>
        `;

        this.curStep = 0;

        let jsv = { act: 'GET_SYS_CONFIG', payload: 'NONE' };
        this.funcCallNative(JSON.stringify(jsv));
    }

    on_NativeCall(jsV) {
        if (jsV.act == 'GET_SYS_CONFIG_RES') {
            if (jsV.payload == 'NONE') {
                alert('시스템 오류가 발생하였습니다.');
            } else {
                this.sysInfo = jsV.payload;
                if (this.sysInfo.status == "OK") this._setState_step0();
                else if (this.sysInfo.status == "ING") { //아직 진행중... 0.5초 후 재시도
                    setTimeout(function() {
                        let jsv = { act: 'GET_SYS_CONFIG', payload: 'NONE' };
                        this.funcCallNative(JSON.stringify(jsv));                
                    }.bind(this), 500);
                }
                else alert('시스템 오류가 발생하였습니다: ' + this.sysInfo.error);
            }
        } else if (jsV.act == 'SET_SYS_CONFIG_RES') {
            if (jsV.payload == 'OK') {
                // this._setState_done();
                this.onClick_reboot();
            } else {
                //let dom = gDOM("cmns_btn_done");
                //dom.style.display = 'block';

                alert('시스템 설정 도중 오류가 발생하였습니다.');
            }
        }
    }

    _setState_step0() {
        this.curStep = 0;
        let ddid = this.sysInfo.dante_dev_id;
        let dti = this.sysInfo.dante_dev_info;
        if (ddid == 'NONE') ddid = 'REMOTE_ID1';

        let v1 = (dti.software_version >> 24);
        let v2 = (dti.software_version >> 16) & 0xFF;
        let v3 = dti.software_version & 0xFFFF;
        let szSWVER = `${v1}.${v2}.${v3}(build: ${dti.software_build})`;
        v1 = (dti.firmware_version >> 24);
        v2 = (dti.firmware_version >> 16) & 0xFF;
        v3 = dti.firmware_version & 0xFFFF;
        let szFWVER = `${v1}.${v2}.${v3}(build: ${dti.firmware_build})`;
        v1 = (dti.bootloader_version >> 24);
        v2 = (dti.bootloader_version >> 16) & 0xFF;
        v3 = dti.bootloader_version & 0xFFFF;
        let szBTLVER = `${v1}.${v2}.${v3}(build: ${dti.bootloader_build})`;


        let html = `
            <div class="flex items-center justify-center flex-col min-h-screen">
                <div class="mb-[20px]">
                    <img src="./assets/images/settingLogo.png" />
                    <div class="text-2xl text-center mt-[20px]">REMOTE 시스템 기본 설정</div>
                </div>
                <div class="flex" id="multi-type">
                    <div class="w-[400px]">
                        <div class="bg-[#343437] rounded-[8px] px-[20px] py-[12px] mb-[12px]">
                            <div class="flex justify-between items-center">
                                <div class="text-sm">DANTE Device ID</div>
                                <input
                                    type="text"
                                    id="rsetup_input_dante_id"
                                    class="input-reset text-sm text-right"
                                    value="${ddid}"
                                    placeholder="Remote 01"
                                />
                            </div>
                        </div>
                        <div class="mb-[4px] text-lg">DANTE 하드웨어 정보</div>
                        <div class="bg-[#343437] rounded-[8px] px-[20px] py-[12px]">
                            <div class="flex justify-between items-center pb-[12px]">
                                <div class="text-sm">Model ID :</div>
                                <div class="text-sm">${dti.model_id}</div>
                            </div>
                            <div class="flex justify-between items-center pb-[12px]">
                                <div class="text-sm">Device ID :</div>
                                <div class="text-sm">${dti.device_id}</div>
                            </div>
                            <div class="flex justify-between items-center pb-[12px]">
                                <div class="text-sm">Software Version :</div>
                                <div class="text-sm">${szSWVER}</div>
                            </div>
                            <div class="flex justify-between items-center pb-[12px]">
                                <div class="text-sm">Firmware Version :</div>
                                <div class="text-sm">${szFWVER}</div>
                            </div>
                            <div class="flex justify-between items-center pb-[12px]">
                                <div class="text-sm">Bootloader Version :</div>
                                <div class="text-sm">${szBTLVER}</div>
                            </div>
                            <div class="flex justify-between items-center pb-[12px]">
                                <div class="text-sm">API Version :</div>
                                <div class="text-sm">${dti.api_version.toString(16)}</div>
                            </div>
                            <div class="flex justify-between items-center pb-[12px]">
                                <div class="text-sm">Capa&Status Flag :</div>
                                <div class="text-sm">${dti.capa_flags.toString(16)} & ${dti.status_flags.toString(16)}</div>
                            </div>
                        </div>
                    </div>
                </div>
                <div class="flex justify-center items-center mt-[20px]">
                    <div id="rsetup_btn_prev" class="bg-[#454548] px-[80px] py-[12px] rounded-[8px] cursor-pointer">이전</div>
                    <div id="reset-btn" class="bg-[#1573d0] px-[80px] py-[12px] rounded-[8px] cursor-pointer ml-[20px]">다음</div>
                </div>
            </div>

            <div id="modal-reset" class="relative z-10 hidden" aria-labelledby="modal-title" role="dialog" aria-modal="true">
                <div class="fixed inset-0 bg-gray-500 bg-opacity-75 transition-opacity"></div>
                <div class="fixed inset-0 z-10 w-screen overflow-y-auto">
                    <div class="flex min-h-full items-end justify-center p-4 text-center sm:items-center sm:p-0">
                        <div class="relative transform overflow-hidden rounded-lg shadow-xl transition-all w-[400px] bg-[#343434]">
                            <div class="px-[40px] pt-[40px] pb-[20px]">
                                <div class="sm:flex sm:items-center sm:justify-center">
                                    <div class="mt-3 text-center sm:mt-0 sm:text-center">
                                        <h3 class="font-semibold leading-6 text-2xl" id="modal-title">재시작 후 설치가 마무리됩니다.</h3>
                                        <div class="mt-[16px]">
                                            <p id="reset-count" class="text-sm">10 초 후 재시작됩니다.</p>
                                        </div>
                                    </div>
                                </div>
                            </div>
                            <div class="px-[40px] py-[20px] sm:flex sm:flex-row-reverse">
                                <button
                                    id="rsetup_btn_next"
                                    type="button"
                                    class="inline-flex w-[180px] justify-center rounded-md bg-[#1573d0] px-3 py-2 text-sm font-semibold text-white shadow-sm sm:ml-3"
                                >
                                    바로 재시작
                                </button>
                                <button
                                    type="button"
                                    id="reset-cancel-btn"
                                    class="mt-3 inline-flex w-[120px] justify-center rounded-md bg-[#343437] px-3 py-2 text-sm font-semibold shadow-sm ring-1 ring-inset ring-gray-100 sm:mt-0"
                                >
                                    취소
                                </button>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        `;

        //console.log(html);
        gDOM('main_area').innerHTML = html;
        gDOM('rsetup_btn_prev').addEventListener('click', this.onClick_changeStep.bind(this, false));
        gDOM('rsetup_btn_next').addEventListener('click', this.onClick_changeStep.bind(this, true));
        gDOM('reset-btn').addEventListener('click', this.onClick_start_countdown.bind(this, true));
        gDOM('reset-cancel-btn').addEventListener('click', this.onClick_start_countdown.bind(this, false));
    }

    _setState_done() {
        this.curStep = 1;
        let html = `
            <center><b>REMOTE 시스템 설정</b></center><br/><br/>
            <center>시스템 설정이 완료되었습니다.<br/>시스템을 재부팅 하세요.</center><br/><br/>
            <center><button id="rsetup_reboot">재부팅</button></center>
        `;

        //console.log(html);
        gDOM('main_area').innerHTML = html;
        gDOM('rsetup_reboot').addEventListener('click', this.onClick_reboot.bind(this));
    }

    _countdown() {
        let countdownTime = 10;
        let countdownElement = document.getElementById('reset-count');

        this.countdownInterval = setInterval(() => {
            countdownElement.innerHTML = countdownTime + ' 초 후 재시작됩니다.';
            countdownTime--;

            if (countdownTime < 0) {
                clearInterval(this.countdownInterval);
                countdownElement.innerHTML = '재부팅 시도중...';
                this.onClick_changeStep(true);
            }
        }, 1000);
    }

    onClick_changeStep(bNext) {
        if (bNext) {
            if (this.curStep == 0) {
                let dom = gDOM('rsetup_input_dante_id');
                if (dom.value.length < 1 || dom.value == 'REMOTE XX:XX:XX') {
                    alert('Dante device id를 입력하세요');
                    return;
                }
                this.sysInfo.dante_dev_id = dom.value;
                gDOM('rsetup_btn_next').style.display = 'none';

                let jsv = { act: 'SET_SYS_CONFIG', payload: this.sysInfo };
                this.funcCallNative(JSON.stringify(jsv));
            }
        } else {
            if (this.curStep <= 0) {
                SwitchPage('common_setup');
            } else {
                // this._setState_step0();
                this.onClick_reboot();
            }
        }
    }

    onClick_reboot() {
        // let dom = gDOM('rsetup_reboot');
        // dom.style.display = 'none'; //더블클릭 방지

        let jsv = { act: 'SYS_REBOOT', payload: 'NONE' };
        this.funcCallNative(JSON.stringify(jsv));
    }

    onClick_start_countdown(val) {
        if (val) {
            document.getElementById('modal-reset').classList.remove('hidden');
            this._countdown();
        } else {
            clearInterval(this.countdownInterval);
            document.getElementById('reset-count').innerHTML = '10 초 후 재시작됩니다.';
            document.getElementById('modal-reset').classList.add('hidden');
        }
    }
};

export { remoteSetup };
