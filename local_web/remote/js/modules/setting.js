const SettingMain = class setting_main {
    constructor(funcCallNative, connector) {
        this.funcCallNative = funcCallNative;        
        this.connector = connector;
        this.ADC = [0, 0];
        this.volt_timer = null;
    }

    BuildLayout() {
        let ddid = this.connector.dante_dev_id;
        let dti = this.connector.sysInfo.dante_dev_info;        

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
                <div class="flex" id="multi-type">
                    <div class="w-[400px]">
                        <div class="bg-[#343437] rounded-[8px] px-[20px] py-[12px] mb-[12px]">
                            <div class="flex justify-between items-center">
                                <div class="text-sm">DANTE Device ID: </div>
                                <div class="text-sm">${ddid}</div>
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
                        <div class="mb-[4px] text-lg">네트워크 정보</div>
                        <div class="bg-[#343437] rounded-[8px] px-[20px] py-[12px]">
                            <div class="flex justify-between items-center pb-[12px]">
                                <div class="text-sm">Interface/Type :</div>
                                <div class="text-sm">${this.connector.baseInfo.network.interface} / ${this.connector.baseInfo.network.type}</div>
                            </div>
                            <div class="flex justify-between items-center pb-[12px]">
                                <div class="text-sm">IP Address :</div>
                                <div class="text-sm">${this.connector.baseInfo.network.cur_ip}</div>
                            </div>
                        </div>                        
                        <div class="mb-[4px] text-lg">시스템 전압</div>
                        <div class="bg-[#343437] rounded-[8px] px-[20px] py-[12px]">
                            <div class="flex justify-between items-center pb-[12px]">
                                <div class="text-sm">12Volt :</div>
                                <div class="text-sm" id="setting_sys_volt_12">${this.ADC[0]} mV</div>
                            </div>
                            <div class="flex justify-between items-center pb-[12px]">
                                <div class="text-sm">3.3Volt :</div>
                                <div class="text-sm" id="setting_sys_volt_33">${this.ADC[1]} mV</div>
                            </div>
                        </div>                        
                        <div class="mb-[4px] text-lg"> </div>
                    </div>
                </div>
                <div class="flex justify-between items-center pb-[12px]">
                    <div id="setting-iotest-btn" class="bg-[#1573d0] px-[20px] py-[12px] rounded-[8px] cursor-pointer ml-[20px]">출력신호 테스트</div>
                    <div id="setting-reset-btn" class="bg-[#1573d0] px-[20px] py-[12px] rounded-[8px] cursor-pointer ml-[20px]">시스템 재 설정</div>                                    
                </div>
                
            </div>
        `;

        gDOM(DIVID_MAINCTS).innerHTML = html;
        gDOM("setting-iotest-btn").addEventListener("click", this._showIOTestModal.bind(this));
        gDOM("setting-reset-btn").addEventListener("click", this._onSysReset.bind(this));

        this._startVoltageTimer();
    }

    on_NativeCall(jsV) {        
        if (jsV.act == "GET_GPIO_VALUES_RES") {
            if (jsV.payload != "NONE") {                
                for (let pinv of jsV.payload) {
                    if (pinv.pin == 13) {
                        this.ADC[0] = parseInt(pinv.value * 1800 / 1023);
                    }
                    else if (pinv.pin == 14) {
                        this.ADC[1] = parseInt(pinv.value * 1800 / 1023);                        
                    }
                    else {
                        console.error("Unknonw pin: " + JSON.stringify(pinv));
                    }
                }
                gDOM("setting_sys_volt_12").innerHTML = this.ADC[0] + " mV";
                gDOM("setting_sys_volt_33").innerHTML = this.ADC[1] + " mV";
            }
            else console.error(jsV.error);
        }
        else if(jsV.act == "SET_GPIO_VALUES_RES") {
            console.log("GPIO_SET_RES: " + JSON.stringify(jsV));
        }
    }

    onDisabled() {
        if (this.volt_timer != null) {
            clearInterval(this.volt_timer);
            this.volt_timer = null;
        }
    }

    _showYesNoModal(szTitle, szAlert, fnYes) {
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
                        <div id="hide_alert_modal_yes" class="i-modal-list-btn-close bg-[#1573d0]">확인</div>
                        <div class="i-modal-list-title">&nbsp;</div>
                        <div id="hide_alert_modal_no" class="i-modal-list-btn-close">취소</div>
                    </div>
                </div>
            </div>
        `;

        gDOM(DIVID_MAINCTS_MODAL).innerHTML = html;
        gDOM('hide_alert_modal_yes').addEventListener("click", fnYes);
        gDOM('hide_alert_modal_no').addEventListener("click", this._hideIOTestModal.bind(this));
    }    

    _onSysReset() {
        this._showYesNoModal("시스템 재 설정", "시스템이 초기화 됩니다.<br/>정말로 초기화를 진행하시겠습니까?", this._doSysReset.bind(this));
    }

    _doSysReset() {
        this._hideIOTestModal();
        let jsv = { act: 'SYS_OPEN_SETUP', payload: 'NONE' };        
        this.funcCallNative(JSON.stringify(jsv));
    }


    _startVoltageTimer() {
        if (this.volt_timer == null) {
            this.volt_timer = setInterval(function () {
                let jsv = { act: 'GET_GPIO_VALUES', payload: {pins: [13, 14]} };        
                this.funcCallNative(JSON.stringify(jsv));
            }.bind(this), 1000);
        }
    }

    _hideIOTestModal() {
        gDOM(DIVID_MAINCTS_MODAL).innerHTML = "";
    }

    _onIOTest_gpio(szId, nGPIO) {
        let pinv = {pin: nGPIO, value: 0};        

        let dom = gDOM(szId);
        if (dom.checked) pinv.value = 1;
        else pinv.value = 0;
                        
        let jsv = { act: 'SET_GPIO_VALUES', payload: {pins: [pinv]} };        
        this.funcCallNative(JSON.stringify(jsv));
    }

    _showIOTestModal() {
        let html = "";
        let evtPairList = [];

        html += `
            <div id="modal-iotest" class="relative z-10" aria-labelledby="modal-title" role="dialog" aria-modal="true">
                <div class="fixed inset-0 transition-opacity bg-gray-500 bg-opacity-75"></div>
                <div class="fixed inset-0 z-10 w-screen overflow-y-auto">
                    <div class="flex items-end justify-center min-h-full p-4 text-center sm:items-center sm:p-0">
                        <div class="relative transform overflow-hidden rounded-[8px] shadow-xl transition-all w-[400px] h-[520px]">
                            <div class="flex items-center flex-col w-full h-full bg-[#343434] px-[12px] pt-[12px]">
                                <div class="flex justify-between items-center w-full h-[40px]">
                                    <div class="cursor-pointer basis-3/12" id="hideSoundModalBtn">
                                        <img src="../assets/images/closeOutline.png" />
                                    </div>
                                    <div class="text-lg text-center basis-6/12">신호출력 테스트</div>
                                    <div class="text-right basis-3/12">
                                        <span class="text-sm cursor-pointer" id="closeTestModal">확인</span>
                                    </div>
                                </div>
                                <div class="flex flex-col justify-start w-full h-[400px] overflow-auto custom-scrollbar">
        `;

        evtPairList.push({ id: "hideSoundModalBtn", fn: this._hideIOTestModal.bind(this) });
        evtPairList.push({ id: "closeTestModal", fn: this._hideIOTestModal.bind(this) });

        let signalList = [
            {name: "비상방송", gpio: 4},
            {name: "방송중", gpio: 6},
            {name: "MIC 방송", gpio: 8},
            {name: "음원 방송", gpio: 10}
        ];

        for (let m of signalList) {
            let tid = "iotest_gpio_select_" + m.gpio;                
            let epf = { id: tid, fn: this._onIOTest_gpio.bind(this, tid, m.gpio) };

            html += `
                <div class="flex items-center justify-between w-full py-[8px]">
                    <div class="text-left basis-4/12">
                        ${m.name}
                    </div>
                    <div class="flex">
                        <label class="el-checkbox">
                            <input type="checkbox" id="${epf.id}" name="check" value="option"/>
                            <span class="el-checkbox-style"></span>
                            <span class="ml-[10px]">On/Off</span>
                        </label>
                    </div>
                </div>
            `;
            //console.log("WTF: " + tid);
            evtPairList.push(epf);
        }

        html += `
                            </div>                                
                        </div>
                    </div>
                </div>
            </div>
        </div>
        `;

        gDOM(DIVID_MAINCTS_MODAL).innerHTML = html;

        for (let epf of evtPairList) gDOM(epf.id).addEventListener("click", epf.fn);        

        //GPIO초기화
        let pins = [{pin: 4, value: 0}, {pin: 6, value: 0}, {pin: 8, value: 0}, {pin: 10, value: 0}];        
        let jsv = { act: 'SET_GPIO_VALUES', payload: {pins: pins} };        
        this.funcCallNative(JSON.stringify(jsv));
    }    
}

export { SettingMain };
