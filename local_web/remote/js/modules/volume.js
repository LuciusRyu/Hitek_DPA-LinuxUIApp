import { PentaVolumeSlide } from "./penta_volume_slider.js";

const VolumeMain = class volume_main {
    constructor(funcCallNative, connector) {
        this.funcCallNative = funcCallNative;
        this.connector = connector;
        this.myTXDev = null;
        this.sliders = null;
    }

    BuildLayout() {
        if (this.myTXDev == null) this.myTXDev = this.connector.getMyTXDev();
        let i;
        let html = `<div class="flex justify-center h-[716px]">`;

        if (this.myTXDev != null) {
            for (i = 0; i < this.myTXDev.tx_channels.length + 1; i++) {
                let szCHName = '';
                if (i == 0) szCHName = "음원 출력";
                else {
                    let txc = this.myTXDev.tx_channels[i - 1];
                    szCHName = txc.nick_name;
                }
                html += `
                    <div class="w-[200px] h-full mx-[8px]">
                        <div class="flex justify-center items-center h-full bg-[#232326] rounded-[8px] px-[20px] pt-[20px] pb-[50px]">
                            <div class="h-full w-[32px] mr-[12px]">
                                <div class="h-[calc(100%-560px)]">
                                    <div class="truncate w-[100px] text-[18px]">${szCHName}</div>
                                </div>
                                <div class="flex flex-col justify-between h-[560px]">
                                    <div class="border-b-4 border-white"></div>
                                    <div class="h-full"></div>
                                    <div class="border-b-4 border-white"></div>
                                    <div class="h-full"></div>
                                    <div class="border-b-4 border-white"></div>
                                    <div class="h-full"></div>
                                    <div class="border-b-4 border-white"></div>
                                    <div class="h-full"></div>
                                    <div class="border-b-4 border-white"></div>
                                    <div class="h-full"></div>
                                    <div class="border-b-4 border-white"></div>
                                    <div class="h-full"></div>
                                    <div class="border-b-4 border-white"></div>
                                    <div class="h-full"></div>
                                    <div class="border-b-4 border-white"></div>
                                    <div class="h-full"></div>
                                    <div class="border-b-4 border-white"></div>
                                    <div class="h-full"></div>
                                    <div class="border-b-4 border-white"></div>
                                    <div class="h-full"></div>
                                    <div class="border-b-4 border-white"></div>
                                </div>
                            </div>
                            <div class="h-full w-[60px]">
                                <div class="h-[calc(100%-560px)]"></div>
                                <div class="h-[560px]">
                                    <div class="w-full h-full" id="slide_${i}"></div>
                                </div>
                            </div>
                            <div class="h-full w-[32px] ml-[12px]">
                                <div id="vm_attr_cur_${i}" class="h-[calc(100%-560px)]"></div>
                                <div class="flex flex-col justify-between h-[560px]">
                                    <div class="border-b-4 border-white"></div>
                                    <div class="flex justify-end items-end text-[14px] h-full">90</div>
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
                                            <div class="flex items-end w-6/12">80</div>
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
                                            <div class="flex items-end w-6/12">70</div>
                                        </div>
                                    </div>
                                    <div class="border-b-4 border-white"></div>
                                    <div class="flex justify-end items-end text-[14px] h-full">60</div>
                                    <div class="border-b-4 border-white"></div>
                                    <div class="flex justify-end items-end text-[14px] h-full">50</div>
                                    <div class="border-b-4 border-white"></div>
                                    <div class="flex justify-end items-end text-[14px] h-full">40</div>
                                    <div class="border-b-4 border-white"></div>
                                    <div class="flex justify-end items-end text-[14px] h-full">30</div>
                                    <div class="border-b-4 border-white"></div>
                                    <div class="flex justify-end items-end text-[14px] h-full">20</div>
                                    <div class="border-b-4 border-white"></div>
                                    <div class="flex justify-end items-end text-[14px] h-full">10</div>
                                    <div class="border-b-4 border-white"></div>
                                    <div class="flex justify-end items-end text-[14px] h-full">0</div>
                                    <div class="border-b-4 border-white"></div>
                                </div>
                            </div>
                        </div>
                    </div>
                `;
            }
        }

        html += `</div>`;

        gDOM(DIVID_MAINCTS).innerHTML = html;

        setTimeout(() => {
            this._setSlider();
            this._getMainboardVolume();
            this._getHardwareInfo(true);
            this._getHardwareInfo(false);
        }, 100);
    }

    on_NativeCall(jsV) {
        if (jsV.act == "GET_HARDWARE_INFO_RES") {
            if (jsV.payload == "NONE") {
                console.error("GET_HARDWARE_INFO failed: " + jsV.error);
                return;
            }
            if (jsV.payload.type == "adc_attr") {
                //attrs = [{channel: 0, attribute: 0}, ... ]
                let attrs = jsV.payload.attributes;
                if (attrs == null) {
                    console.error("Get Channel attributes failed...");
                    return;
                }
                this._setChannelAttr(attrs);
            } else if (jsV.payload.type == "adc_volume") {
                //volumes = [{channel: 0, volume: 0}, ... ]                
                let volumes = jsV.payload.volumes;
                let cVolumes = [];
                if (volumes == null) {
                    console.error("Get Channel volume failed...");
                    return;
                }

                console.log("Received volume:\n" + JSON.stringify(volumes));
                for (let v of volumes) {
                    let cv = this._mainCHVolConversion(true, v.volume);
                    cVolumes.push({ channel: v.channel, volume: cv / 100.0 });
                }

                this._setChannelVolume(cVolumes);
            } else {
                console.error("Unkown type...");
                console.log(JSON.stringify(jsV));
            }
        } 
        else if (jsV.act == "SET_HARDWARE_INFO_RES") {
            console.log("Set Hardware info done!");
            console.log(JSON.stringify(jsV));
        }
        else if (jsV.act == "SYS_RUN_COMMAND") {
            console.log("Sys run command");
            console.log(JSON.stringify(jsV));
        }
    }

    _getHardwareInfo(bIsVolume) {
        let reqV = { act: "GET_HARDWARE_INFO", payload: "adc_attr" };
        if (bIsVolume) reqV.payload = "adc_volume";
        this.funcCallNative(JSON.stringify(reqV));
    }

    _getMainboardVolume() {
        let cmdPayload = { cmd: "amixer get Master", cmd_id: 100, need_result: true };
        let jsv = { act: "SYS_RUN_COMMAND", payload: cmdPayload };
        this.funcCallNative(JSON.stringify(jsv));
    }

    _onVolumeSet(idx) {
        if (idx > 0) {
            let vol = [];
            let cVolume = document.querySelector(`#slide_${idx}_sld_btn > div`).innerText;        
            vol.push({ channel: idx - 1, volume: this._mainCHVolConversion(false, cVolume) });
            console.log("Set Volume:\n" + JSON.stringify(vol));

            let reqV = { act: "SET_HARDWARE_INFO", payload: { type: "adc_volume", value: vol } };        
            this.funcCallNative(JSON.stringify(reqV));
        }
    }

    _setChannelAttr(attrs) {
        //attrs = [{channel: 0, attribute: 0}, ... ]
        let dom;
        for (let attr of attrs) {
            dom = gDOM("vm_attr_cur_" + attr.channel);
            if (dom == null) {
                console.log("ERROR: Channel for attr is not exist: " + attr.channel);
                continue;
            }
            // dom.innerText = " " + attr.attribute;
            if (attr.channel > 0) {
                dom.innerHTML = `<div class="w-full text-center p-[5px] rounded-[8px] text-[12px] bg-[#1573d0]">AGC</div>`;
            } else {
                dom.innerHTML = ``;
            }
        }
    }

    _setChannelVolume(volumes) {
        //volumes = [{channel: 0, volume: 0}, ... ]
        let dom;
        for (let vol of volumes) {
            // dom = gDOM("vm_volume_cur_" + vol.channel);
            dom = document.querySelector(`#slide_${vol.channel + 1}_sld_btn > div`);
            if (dom == null) {
                console.log("ERROR: Channel for volume is not exist: " + vol.channel);
                continue;
            }
            dom.innerText = vol.volume;
            //console.log(JSON.stringify(vol));
            this.sliders[vol.channel + 1].SetValue(vol.volume);
        }
    }

    _setSlider() {
        let i;
        this.sliders = new Array();
        for (i = 0; i < this.myTXDev.tx_channels.length + 1; i++) {
            this.sliders.push(new PentaVolumeSlide(`slide_${i}`, 0, 1));
        }

        for (i = 0; i < this.sliders.length; i++) {
            this.sliders[i].Show();
            this.sliders[i].SetOnChangeCallback(this._onVolumeSet.bind(this, i));
        }
    }

    _mainCHVolConversion(bToPercent, tVal) {
        //PGA Gain 0에서 120단계로 증폭, 0.5db단위
        //120 ~ 127은 동일
        //최상위 비트는 Mute
        let res = parseInt(tVal);

        if (bToPercent === true) {
            if (res < 0) res = 0;
            if (res > 127) res = 0;
            else if (res > 120) res = 120;
            
            res = parseInt((100 * res) / 120, 10);
            if (res > 100) res = 100;
        } else {
            if (res <= 0) {
                res = 128;
            }
            else {
                if (res > 100) res = 100;
                res = parseInt((120 * res) / 100);
            }
        }

        return res;
    }
};

export { VolumeMain };
