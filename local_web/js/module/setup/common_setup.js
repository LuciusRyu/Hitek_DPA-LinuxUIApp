const commonSetup = class common_setup {
    constructor(funcCallNative) {
        this.page_name = 'Common_setup';
        this.funcCallNative = funcCallNative;
        this.sysInfo = null;
    }

    /*
    {
        "network":
            {
                "interface":"eth0",
                "cur_ip":"192.168.20.62",
                "static_gateway":"192.168.20.1",
                "static_ip":"192.168.20.58",
                "static_netmask":"255.255.255.0",
                "type":"STATIC"
            },
        "status":0,
        "sys_id":"NONE",
        "sys_type":"REMOTE"
    }    
    */

    SetBaseInfo(baseInfo) {
        this.sysInfo = baseInfo;
        //console.log(JSON.stringify(baseInfo));
        if (this.sysInfo.network.type != 'STATIC') {
            //DHCP면 아래 정보가 없다
            this.sysInfo.network.static_ip = '192.168.0.2';
            this.sysInfo.network.static_netmask = '255.255.255.0';
            this.sysInfo.network.static_gateway = '192.168.0.1';
        }
    }

    BuildLayout() {
        let titleTxt = '알수 없는 시스템 기본 설정';
        if (this.sysInfo.sys_type == 'MAIN_TX') titleTxt = 'MAIN TX 시스템 기본 설정';
        else if (this.sysInfo.sys_type == 'REMOTE') titleTxt = 'REMOTE 시스템 기본 설정';
        else if (this.sysInfo.sys_type == 'INT_CNC') titleTxt = '통합관제 시스템 기본 설정';

        let sysID = this.sysInfo.sys_id;
        if (sysID == 'NONE') {
            if (this.sysInfo.sys_type == 'MAIN_TX') sysID = 'MAIN_TX_ID1';
            else if (this.sysInfo.sys_type == 'REMOTE') sysID = 'REMOTE_ID1';
            else if (this.sysInfo.sys_type == 'INT_CNC') sysID = 'CNCSRV_ID1';
        }

        let ipTypeD = '';
        let ipTypeS = '';
        let ipSCls = 'hidden';
        if (this.sysInfo.network.type == 'STATIC') {
            ipTypeS = 'checked';
            ipSCls = '';
        } else ipTypeD = 'checked';

        let html = `
            <div class="flex items-center justify-center flex-col min-h-screen">
                <div class="mb-[20px]">
                    <img src="./assets/images/settingLogo.png" />
                    <div class="text-2xl text-center mt-[20px]">${titleTxt}</div>
                </div>
                <div class="mb-[20px] w-[400px]">
                    <div class="bg-[#343437] rounded-[8px] px-[20px] py-[12px] mb-[12px]">
                        <div class="flex justify-between items-center">
                            <div class="text-sm">시스템 ID</div>
                            <input type="text" id="cmns_input_sysid" class="input-reset text-sm text-right" value="${sysID}" placeholder="Main tx" />
                        </div>
                    </div>
                    <div class="mb-[4px] text-lg">네트워크</div>
                    <div class="bg-[#343437] rounded-[8px] px-[20px] py-[12px]">
                        <div class="flex justify-between items-center pb-[12px]">
                            <div class="text-sm">주인터페이스 지정</div>
                            <input type="text" id="cmns_input_net_inf" class="input-reset text-sm text-right" value="${this.sysInfo.network.interface}" placeholder="eth0" />
                        </div>
                        <div class="flex justify-between items-center py-[12px]">
                            <div class="text-sm">방송 포트 번호</div>
                            <input
                                type="text"
                                id="cmns_input_net_broadcast"
                                class="input-reset text-sm text-right"
                                oninput="this.value = this.value.replace(/[^0-9]/g, '');"
                                value="${this.sysInfo.network.broadcast_port}"
                                placeholder="0"
                            />
                        </div>
                        <div class="flex justify-between items-center py-[12px]">
                            <div class="text-sm">현재 IP주소</div>
                            <div class="text-sm">${this.sysInfo.network.cur_ip}</div>
                        </div>
                        <div class="flex justify-between items-center pt-[12px]">
                            <div class="text-sm">주소 지정 방식</div>
                            <div class="flex justify-end items-center">
                                <div id="cmns_chk_net_dhcp" class="el-radio flex justify-between items-center mr-[18px]">
                                    <input type="radio" name="addressingType" id="dhcp-chk" value="option" ${ipTypeD} />
                                    <label class="el-radio-style" for="dhcp-chk"></label>
                                    <span class="margin-r ml-[10px]">DHCP</span>
                                </div>
                                <div id="cmns_chk_net_static" class="el-radio flex justify-between items-center">
                                    <input type="radio" name="addressingType" id="static-chk" value="option" ${ipTypeS} />
                                    <label class="el-radio-style" for="static-chk"></label>
                                    <span class="margin-r ml-[10px]">고정 IP</span>
                                </div>
                            </div>
                        </div>
                        <div class="${ipSCls}" id="address-setting-box">
                            <div class="flex justify-between items-center pt-[24px] pb-[12px]" id="address-setting-box-ip">
                                <div class="text-sm">IP주소</div>
                                <input
                                    type="text"
                                    id="cmns_input_net_static_ip"
                                    class="input-reset text-sm text-right"
                                    value="${this.sysInfo.network.static_ip}"
                                    oninput="this.value = this.value.replace(/[^0-9.]/g, '');"
                                    placeholder="0.0.0.0"
                                />
                            </div>
                            <div class="flex justify-between items-center py-[12px]" id="address-setting-box-mask">
                                <div class="text-sm">서브넷 마스크</div>
                                <input
                                    type="text"
                                    id="cmns_input_net_static_netmask"
                                    class="input-reset text-sm text-right"
                                    value="${this.sysInfo.network.static_netmask}"
                                    oninput="this.value = this.value.replace(/[^0-9.]/g, '');"
                                    placeholder="0.0.0.0"
                                />
                            </div>
                            <div class="flex justify-between items-center pt-[12px]" id="address-setting-box-gate">
                                <div class="text-sm">게이트웨이</div>
                                <input
                                    type="text"
                                    id="cmns_input_net_static_gateway"
                                    class="input-reset text-sm text-right"
                                    value="${this.sysInfo.network.static_gateway}"
                                    oninput="this.value = this.value.replace(/[^0-9.]/g, '');"
                                    placeholder="0.0.0.0"
                                />
                            </div>
                        </div>
                    </div>
                </div>
                <div
                    id="cmns_btn_done"
                    class="flex items-center justify-center bg-[#1573d0] rounded-[8px] text-sm text-center w-[400px] h-[44px] cursor-pointer"
                >
                    다음
                </div>
            </div>        
        `;

        //console.log(html);
        gDOM('main_area').innerHTML = html;
        gDOM('cmns_chk_net_dhcp').addEventListener('click', this.onClick_iptypeChange.bind(this, false));
        gDOM('cmns_chk_net_static').addEventListener('click', this.onClick_iptypeChange.bind(this, true));
        gDOM('cmns_btn_done').addEventListener('click', this.onClick_proceedNext.bind(this));

        //스크립트 실행
        if (this.sysInfo.sys_type == 'MAIN_TX') {
            let tcmd = "xfce4-terminal -e '/HitekDPA/scripts/check_and_prepare.sh'";
            let reqV = { act: "SYS_RUN_COMMAND", payload: { cmd_id: "1", cmd: tcmd } };
            this.funcCallNative(JSON.stringify(reqV));
        }
    }

    on_NativeCall(jsV) {
        if (jsV.act == 'MODIFY_BASEINFO_DONE') {
            if (jsV.payload == 'OK') {
                if (this.sysInfo.sys_type == 'MAIN_TX') SwitchPage('setup_main_tx');
                else if (this.sysInfo.sys_type == 'REMOTE') SwitchPage('setup_remote');
                else if (this.sysInfo.sys_type == 'INT_CNC') SwitchPage('setup_int_cnc');
            } else {
                let dom = gDOM('cmns_btn_done');
                dom.style.display = 'block';

                alert('시스템 설정 도중 오류가 발생하였습니다.');
            }
        }
    }

    onClick_proceedNext() {
        let dom = gDOM('cmns_input_sysid');
        if (dom.value.length < 1) {
            alert('시스템 아이디를 입력하세요');
            return;
        }
        this.sysInfo.sys_id = dom.value;
        dom = gDOM('cmns_input_net_inf');
        if (dom.value.length < 1) {
            alert('네트워크 인터페이스를 입력하세요');
            return;
        }
        this.sysInfo.network.interface = dom.value;
        dom = gDOM('cmns_input_net_broadcast');
        this.sysInfo.network.broadcast_port = parseInt(dom.value);
        dom = gDOM('static-chk');
        if (dom.checked) {
            this.sysInfo.network.type = 'STATIC';
            dom = gDOM('cmns_input_net_static_ip');
            if (dom.value.length < 8) {
                alert('고정IP를 입력하세요');
                return;
            }
            this.sysInfo.network.static_ip = dom.value;
            dom = gDOM('cmns_input_net_static_netmask');
            if (dom.value.length < 8) {
                alert('고정IP NETMASK를 입력하세요');
                return;
            }
            this.sysInfo.network.static_netmask = dom.value;
            dom = gDOM('cmns_input_net_static_gateway');
            if (dom.value.length < 8) {
                alert('고정IP GATEWAY를 입력하세요');
                return;
            }
            this.sysInfo.network.static_gateway = dom.value;
        } else this.sysInfo.network.type = 'DHCP';

        dom = gDOM('cmns_btn_done');
        dom.style.display = 'none'; //더블클릭 방지

        let jsv = { act: 'MODIFY_BASEINFO', payload: this.sysInfo };
        this.funcCallNative(JSON.stringify(jsv));
    }

    onClick_iptypeChange(bIsStatic) {
        console.log('onClick_iptypeChange', bIsStatic);
        let domStatic = gDOM('static-chk');
        let domDhcp = gDOM('dhcp-chk');
        if (bIsStatic) {
            document.getElementById('address-setting-box').classList.remove('hidden');
            domStatic.checked = true;
            domDhcp.checked = false;
        } else {
            document.getElementById('address-setting-box').classList.add('hidden');
            domStatic.checked = false;
            domDhcp.checked = true;
        }
    }
};

export { commonSetup };
