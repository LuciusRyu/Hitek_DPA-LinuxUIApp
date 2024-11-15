const maintxSetup = class maintx_setup {
    constructor(funcCallNative) {
        this.page_name = "MainTX_Setup";
        this.funcCallNative = funcCallNative;
        this.curStep = 0;
        this.baseInfo = null;
        this.sysInfo = null;
        this.loginInfo = null;
        this.countdownInterval = null;
    }

    SetBaseInfo(baseInfo) {
        this.baseInfo = baseInfo;
    }

    BuildLayout() {
        let html = `
              <center><b>MAIN_TX 시스템 설정</b></center><br/>
              <center>Wait for response .....</center><br/>
        `;
        gDOM("main_area").innerHTML = html;

        this.curStep = 0;

        let reqV = { act: "GET_SYS_CONFIG", payload: "NONE" };
        this.funcCallNative(JSON.stringify(reqV));
    }

    on_NativeCall(jsV) {
        if (jsV.act == "GET_SYS_CONFIG_RES") {
            if (jsV.payload == "NONE") {
                alert("시스템 오류가 발생하였습니다.");
            } else {
                this.sysInfo = jsV.payload;
                // console.log(JSON.stringify(this.sysInfo));
                this._setState_step0();
            }
        } else if (jsV.act == "SET_SYS_CONFIG_RES") {
            if (jsV.payload == "OK") {
                if (this.curStep === 0) {
                    this._setState_step1();
                } else if (this.curStep === 1) {
                    let reqV = { act: "GET_LOGIN_INFO", payload: "NONE" };
                    this.funcCallNative(JSON.stringify(reqV));
                } else {
                    gDOM("msetup_btn_next").style.display = "block";
                    alert("시스템 설정 도중 오류가 발생하였습니다.");
                }
            } else {
                gDOM("msetup_btn_next").style.display = "block";
                alert("시스템 설정 도중 오류가 발생하였습니다.");
            }
        } else if (jsV.act == "GET_LOGIN_INFO_RES") {
            if (jsV.payload == "NONE") {
                gDOM("msetup_btn_next").style.display = "block";
                alert("시스템 오류가 발생하였습니다.");
            } else {
                this.loginInfo = jsV.payload;
                //console.log(JSON.stringify(this.sysInfo));
                this._setState_step2();
            }
        } else if (jsV.act == "SET_LOGIN_INFO_RES") {
            if (jsV.payload == "OK") {
                // this._setState_done();
                this._setState_step3();
                //let reqV = { act: 'SYS_RUN_COMMAND', payload: { cmd_id: '1', cmd: "xfce4-terminal -e '/bin/bash ls'" } };
                let tcmd = "xfce4-terminal -e '/HitekDPA/scripts/install_by_setup.sh " + this.sysInfo.dante_dev_id;
                if (this.sysInfo.use_vrrp) {
                    if (this.sysInfo.vrrp.is_master) tcmd += " master";
                    else tcmd += " backup";
                    tcmd += " " + this.sysInfo.vrrp.ip;
                } else tcmd += " single";
                tcmd += "'";

                let reqV = { act: "SYS_RUN_COMMAND", payload: { cmd_id: "1", cmd: tcmd } };
                this.funcCallNative(JSON.stringify(reqV));
            } else {
                gDOM("msetup_btn_next").style.display = "block";
                alert("시스템 설정 도중 오류가 발생하였습니다.");
            }
        } else if (jsV.act == "SYS_RUN_COMMAND_RES") {
            // this._setState_done();
            this.onClick_reboot();
        }
    }

    _setState_step0() {
        this.curStep = 0;
        let ddid = this.sysInfo.dante_dev_id;
        if (ddid == "NONE") ddid = "DTX-sixhex";

        let unusedChk = "checked";
        if (this.sysInfo.use_vrrp) {
            unusedChk = "";
        } else {
            this.sysInfo.vrrp = {
                id: 1,
                is_master: true,
                priority: 100,
                interface: "eth0",
                ip: "192.168.0.100",
                clusters: [],
            };
        }

        let html = `
            <div class="flex items-center justify-center flex-col min-h-screen">
                <div class="mb-[20px]">
                    <img src="./assets/images/settingLogo.png" />
                    <div class="text-2xl text-center mt-[20px]">다중화 설정</div>
                </div>
                <div class="flex" id="multi-type">
                    <div class="w-[400px]">
                        <div class="bg-[#343437] rounded-[8px] px-[20px] py-[12px] mb-[12px]">
                            <div class="flex justify-between items-center pb-[12px]">
                                <div class="text-sm">DANTE Device ID</div>
                                <input
                                    type="text"
                                    id="msetup_input_dante_id"
                                    class="input-reset text-sm text-right"
                                    value="${ddid}"
                                    placeholder="Main TX 01"
                                />
                            </div>                        
                            <div class="flex justify-between items-center pt-[6px]">
                                <div class="text-sm">시스템 다중화</div>
                                <div class="flex justify-between items-center">
                                    <label class="el-switch">
                                        <input
                                            id="msetup_vrrp_use"
                                            type="checkbox"
                                            name="switch"
                                            ${unusedChk}
                                        />
                                        <span class="el-switch-style"></span>
                                    </label>
                                </div>
                            </div>
                        </div>
                        <div id="msetup-vrrp-mode-1" style="${unusedChk !== "" ? "display: block;" : "display: none;"}">
                            <div class="mb-[4px] text-lg">주 인터페이스</div>
                            <div class="bg-[#343437] rounded-[8px] px-[20px] py-[12px] mb-[12px]">
                                <div class="flex justify-between items-center pb-[12px]">
                                    <div class="text-sm">지정 네트워크 인터페이스</div>
                                    <div class="text-sm">
                                        ${this.baseInfo.network.interface}
                                    </div>
                                </div>
                                <div class="flex justify-between items-center pt-[12px]">
                                    <div class="text-sm">현재 IP</div>
                                    <div class="text-sm">
                                        ${this.baseInfo.network.cur_ip}
                                    </div>
                                </div>
                            </div>
                            <div class="mb-[4px] text-lg">다중화 시스템</div>
                            <div class="bg-[#343437] rounded-[8px] px-[20px] py-[12px]">
                                <div class="flex justify-between items-center pb-[12px]">
                                    <div class="text-sm">시스템 ID</div>
                                    <input
                                        type="text"
                                        id="msetup_vrrp_sys_id"
                                        class="input-reset text-sm text-right"
                                        value="${this.sysInfo.vrrp.id}"
                                        oninput="this.value = this.value.replace(/[^0-9]/g, '');"
                                        placeholder="0"
                                    />
                                </div>
                                <div class="flex justify-between items-center py-[12px]">
                                    <div class="text-sm">Master/Slave</div>
                                    <div class="flex justify-end items-center">
                                        <div id="msetup_vrrp_chk_master" class="el-radio flex justify-between items-center mr-[18px]">
                                            <input
                                                type="radio"
                                                name="addressingType"
                                                id="master-chk"
                                                value="option"
                                                ${this.sysInfo.vrrp.is_master ? "checked" : ""}
                                            />
                                            <label class="el-radio-style" for="master-chk"></label>
                                            <span class="margin-r ml-[10px]">Master</span>
                                        </div>
                                        <div id="msetup_vrrp_chk_slave" class="el-radio flex justify-between items-center">
                                            <input
                                                type="radio"
                                                name="addressingType"
                                                id="slave-chk"
                                                value="option"
                                                ${this.sysInfo.vrrp.is_master ? "" : "checked"}
                                            />
                                            <label class="el-radio-style" for="slave-chk"></label>
                                            <span class="margin-r ml-[10px]">Slave</span>
                                        </div>
                                    </div>
                                </div>
                                <div class="flex justify-between items-center py-[12px]">
                                    <div class="text-sm">시스템 우선순위</div>
                                    <input
                                        type="text"
                                        id="msetup_vrrp_priority"
                                        class="input-reset text-sm text-right"
                                        value="${this.sysInfo.vrrp.priority}"
                                        oninput="this.value = this.value.replace(/[^0-9]/g, '');"
                                        placeholder="0"
                                    />
                                </div>
                                <div class="flex justify-between items-center py-[12px]">
                                    <div class="text-sm">대상 네트워크 인터페이스</div>
                                    <input
                                        type="text"
                                        id="msetup_vrrp_netif"
                                        class="input-reset text-sm text-right"
                                        value="${this.sysInfo.vrrp.interface}"
                                        placeholder="eth1"
                                    />
                                </div>
                                <div class="flex justify-between items-center pt-[12px]">
                                    <div class="text-sm">서비스 IP</div>
                                    <input
                                        type="text"
                                        id="msetup_vrrp_ip"
                                        class="input-reset text-sm text-right"
                                        value="${this.sysInfo.vrrp.ip}"
                                        oninput="this.value = this.value.replace(/[^0-9.]/g, '');"
                                        placeholder="0.0.0.0"
                                    />
                                </div>
                            </div>
                        </div>
                    </div>
                    <div
                        id="msetup-vrrp-mode-2"
                        class="w-[400px] ml-[20px] bg-[#343437] rounded-[8px] p-[8px]"
                        style="${unusedChk !== "" ? "display: block;" : "display: none;"}"
                    >
                        <div class="flex justify-center items-center flex-col">
                            <div class="flex justify-between items-center w-full mb-[8px]">
                                <div class="basis-3/12"></div>
                                <div class="basis-6/12 text-center text-lg">클러스터 목록</div>
                                <div class="basis-3/12 text-right">
                                    <span class="text-sm cursor-pointer" id="cluster-show-btn">추가</span>
                                </div>
                            </div>
                            <div class="flex justify-between items-center w-full h-[435px] bg-[#232326] rounded-[8px]">
                                <div id="mset_vrrp_clusters" class="w-full h-full overflow-auto custom-scrollbar">
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
                <div class="flex justify-center items-center mt-[20px]">
                    <div id="msetup_btn_prev" class="bg-[#454548] px-[80px] py-[12px] rounded-[8px] cursor-pointer">이전</div>
                    <div id="msetup_btn_next" class="bg-[#1573d0] px-[80px] py-[12px] rounded-[8px] cursor-pointer ml-[20px]">다음</div>
                </div>
            </div>

            <div id="modal-cluster" class="relative z-10 hidden" aria-labelledby="modal-title" role="dialog" aria-modal="true">
                <div class="fixed inset-0 bg-gray-500 bg-opacity-75 transition-opacity"></div>
                <div class="fixed inset-0 z-10 w-screen overflow-y-auto">
                    <div class="flex min-h-full items-end justify-center p-4 text-center sm:items-center sm:p-0">
                        <div class="relative transform overflow-hidden rounded-lg shadow-xl transition-all w-[400px]">
                            <div class="flex justify-center items-center flex-col w-[400px] bg-[#343434] p-[20px]">
                                <div class="flex justify-between items-center w-full mb-[8px]">
                                    <div class="basis-3/12 cursor-pointer" id="cluster-hide-btn">
                                        <img src="./assets/images/closeOutline.png" />
                                    </div>
                                    <div class="basis-6/12 text-center text-lg">클러스터 추가</div>
                                    <div class="basis-3/12 text-right">
                                        <span class="text-sm cursor-pointer" id="meset_cluster_add">확인</span>
                                    </div>
                                </div>
                                <div class="flex justify-between items-center w-full h-[89px] bg-[#232326] rounded-[8px]">
                                    <div class="w-full h-full overflow-auto custom-scrollbar">
                                        <div class="flex justify-between items-center px-[10px] py-[12px] border-b">
                                            <div class="text-sm">이름</div>
                                            <div class="flex justify-center items-center">
                                                <div class="text-sm">
                                                    <input
                                                        type="text"
                                                        id="msetup_cluster_id_new"
                                                        class="input-reset text-sm text-right"
                                                        placeholder="eth1"
                                                    />
                                                </div>
                                            </div>
                                        </div>
                                        <div class="flex justify-between items-center px-[10px] py-[12px]">
                                            <div class="text-sm">IP</div>
                                            <div class="flex justify-center items-center">
                                                <div class="text-sm">
                                                    <input
                                                        type="text"
                                                        id="msetup_cluster_ip_new"
                                                        class="input-reset text-sm text-right"
                                                        oninput="this.value = this.value.replace(/[^0-9.]/g, '');"
                                                        placeholder="0.0.0.0"
                                                    />
                                                </div>
                                            </div>
                                        </div>
                                    </div>
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        `;

        //console.log(html);
        gDOM("main_area").innerHTML = html;
        gDOM("msetup_vrrp_use").addEventListener("click", this.onClick_use_vrrp.bind(this));
        gDOM("msetup_vrrp_chk_master").addEventListener("click", this.onClick_vrrp_change_master.bind(this, true));
        gDOM("msetup_vrrp_chk_slave").addEventListener("click", this.onClick_vrrp_change_master.bind(this, false));
        gDOM("msetup_btn_prev").addEventListener("click", this.onClick_changeStep.bind(this, false));
        gDOM("msetup_btn_next").addEventListener("click", this.onClick_changeStep.bind(this, true));
        gDOM("meset_cluster_add").addEventListener("click", this.onClick_cluster_add.bind(this));
        gDOM("cluster-show-btn").addEventListener("click", this.onClick_cluster_modal.bind(this, true));
        gDOM("cluster-hide-btn").addEventListener("click", this.onClick_cluster_modal.bind(this, false));

        if (this.sysInfo.use_vrrp) this._refreshClusters();
    }

    _setState_step1() {
        this.curStep = 1;
        let iName = "";

        if (this.baseInfo.network.interface == "eth0") iName = "eth1";
        else if (this.baseInfo.network.interface == "eth1") iName = "eth0";
        else iName = "eth1";

        let html = `
            <div class="flex flex-col items-center justify-center min-h-screen">
                <div class="mb-[20px]">
                    <img src="./assets/images/settingLogo.png" />
                    <div class="text-2xl text-center mt-[20px]">MAIN TX 시스템 기본 설정</div>
                </div>
                <div class="flex" id="multi-type">
                    <div class="w-[400px]">
                        <div class="bg-[#343437] rounded-[8px] mb-[12px]">
                            <div class="flex items-center justify-between px-[20px] py-[12px]">
                                <div class="text-sm">재난경보 연결</div>
                                <div class="w-[200px]">
                                    <form class="max-w-sm mx-auto" style="margin: 0">
                                        <select
                                            id="msetup_emr_type"
                                            class="bg-[#343437] text-black text-sm rounded-lg block w-full text-center"
                                        >
                                            <option value="NONE" selected>없음</option>
                                            <option value="TCPIP">TCP/IP</option>
                                            <option value="RS232">RS232</option>
                                        </select>
                                    </form>
                                </div>
                            </div>
                        </div>
                        <div id="msetup_emr_tcp" style="display: none">
                            <div class="bg-[#343437] rounded-[8px] mb-[12px]">
                                <div class="flex items-center justify-between px-[20px] py-[12px]">
                                    <div class="text-sm">Network I/F</div>
                                    <div class="flex items-center justify-end">
                                        <div class="el-radio flex justify-between items-center mr-[18px]">
                                            <input
                                                type="radio"
                                                name="networkIF"
                                                id="msetup_emr_if_main"
                                                value="mif"
                                                checked
                                            />
                                            <label class="el-radio-style" for="msetup_emr_if_main"></label>
                                            <span class="margin-r ml-[10px]">Main I/F</span>
                                        </div>
                                        <div class="flex items-center justify-between el-radio">
                                            <input
                                                type="radio"
                                                name="networkIF"
                                                id="msetup_emr_if_fix"
                                                value="fif"
                                            />
                                            <label class="el-radio-style" for="msetup_emr_if_fix"></label>
                                            <span class="margin-r ml-[10px]">I/F 지정</span>
                                        </div>
                                    </div>
                                </div>
                                <div id="msetup_emr_net_type" style="display: none">
                                    <div class="flex justify-between items-center px-[20px] py-[12px]">
                                        <div class="text-sm">인터페이스 명칭</div>
                                        <input type="text" id="msetup_emr_net_name" class="text-sm text-right input-reset" value="${iName}" placeholder="eth0" />
                                    </div>
                                    <div class="flex items-center justify-between px-[20px] py-[12px]">
                                        <div class="text-sm">주소 지정 방식</div>
                                        <div class="flex items-center justify-end">
                                            <div class="el-radio flex justify-between items-center mr-[18px]">
                                                <input
                                                    type="radio"
                                                    name="addressType"
                                                    id="msetup_emr_m_dhcp"
                                                    value="DHCP"
                                                    checked
                                                />
                                                <label class="el-radio-style" for="msetup_emr_m_dhcp"></label>
                                                <span class="margin-r ml-[10px]">DHCP</span>
                                            </div>
                                            <div class="flex items-center justify-between el-radio">
                                                <input
                                                    type="radio"
                                                    name="addressType"
                                                    id="msetup_emr_m_ip"
                                                    value="STATIC"
                                                />
                                                <label class="el-radio-style" for="msetup_emr_m_ip"></label>
                                                <span class="margin-r ml-[10px]">고정 IP</span>
                                            </div>
                                        </div>
                                    </div>
                                </div>
                            </div>
                            <div class="bg-[#343437] rounded-[8px] px-[20px] py-[12px] mb-[12px]">
                                <div class="flex justify-between items-center pb-[12px]">
                                    <div class="text-sm">장비 IP Address</div>
                                    <input type="text" id="msetup_emr_tcp_ip" class="text-sm text-right input-reset" placeholder="192.168.0.0" />
                                </div>
                                <div class="flex justify-between items-center pt-[12px]">
                                    <div class="text-sm">장비 Port</div>
                                    <input type="text" id="msetup_emr_tcp_port" class="text-sm text-right input-reset" placeholder="5000" />
                                </div>
                            </div>
                        </div>
                        <div id="msetup_emr_rs" style="display: none">
                            <div class="bg-[#343437] rounded-[8px] px-[20px] py-[12px] mb-[12px]">
                                <div class="flex justify-between items-center pb-[12px]">
                                    <div class="text-sm">장치명</div>
                                    <input type="text" id="msetup_emr_rs_name" class="text-sm text-right input-reset" placeholder="/dev/ttyS4" />
                                </div>
                                <div class="flex justify-between items-center pt-[12px]">
                                    <div class="text-sm">속도</div>
                                    <input type="text" id="msetup_emr_rs_rate" class="text-sm text-right input-reset" placeholder="115200" />
                                </div>
                            </div>
                        </div>
                    </div>
                    <div id="msetup_emr_cluster" class="w-[400px] ml-[20px]" style="display: none">
                        <div class="bg-[#343437] rounded-[8px] mb-[12px]">
                            <div class="flex justify-between items-center px-[20px] py-[12px]">
                                <div class="text-sm">IP주소</div>
                                <input type="text" id="msetup_emr_cluster_ip" class="text-sm text-right input-reset" placeholder="192.168.0.2" />
                            </div>
                            <div class="flex justify-between items-center px-[20px] py-[12px]">
                                <div class="text-sm">서브넷 마스크</div>
                                <input type="text" id="msetup_emr_cluster_mask" class="text-sm text-right input-reset" placeholder="255.255.255.0" />
                            </div>
                            <div class="flex justify-between items-center px-[20px] py-[12px]">
                                <div class="text-sm">게이트웨이</div>
                                <input type="text" id="msetup_emr_cluster_gateway" class="text-sm text-right input-reset" placeholder="192.168.0.1" />
                            </div>
                            <div class="flex justify-between items-center px-[20px] py-[12px]">
                                <div class="text-sm">DNS 1</div>
                                <input type="text" id="msetup_emr_cluster_dns1" class="text-sm text-right input-reset" placeholder="8.8.8.8" />
                            </div>
                            <div class="flex justify-between items-center px-[20px] py-[12px]">
                                <div class="text-sm">DNS 2</div>
                                <input type="text" id="msetup_emr_cluster_dns2" class="text-sm text-right input-reset" placeholder="0.0.0.0" />
                            </div>
                        </div>
                    </div>
                </div>
                <div class="flex justify-center items-center mt-[20px]">
                    <div id="msetup_btn_prev" class="bg-[#454548] px-[80px] py-[12px] rounded-[8px] cursor-pointer">이전</div>
                    <div id="msetup_btn_next" class="bg-[#1573d0] px-[80px] py-[12px] rounded-[8px] cursor-pointer ml-[20px]">다음</div>
                </div>
            </div>       
        `;

        gDOM("main_area").innerHTML = html;

        gDOM("msetup_emr_type").addEventListener("change", this.toggle_connection_type.bind(this));
        gDOM("msetup_emr_if_main").addEventListener("click", this.toggle_network_type.bind(this));
        gDOM("msetup_emr_if_fix").addEventListener("click", this.toggle_network_type.bind(this));
        gDOM("msetup_emr_m_ip").addEventListener("click", this.toggle_address_type.bind(this));
        gDOM("msetup_emr_m_dhcp").addEventListener("click", this.toggle_address_type.bind(this));
        gDOM("msetup_btn_prev").addEventListener("click", this.onClick_changeStep.bind(this, false));
        gDOM("msetup_btn_next").addEventListener("click", this.onClick_changeStep.bind(this, true));
    }

    _setState_step2() {
        this.curStep = 2;

        let autoChk = "";
        if (this.loginInfo.auto) autoChk = "checked";
        let loginID = this.loginInfo.id;
        if (loginID == "NONE") {
            loginID = "local@localhost";
            this.loginInfo.id = loginID;
        }

        let html = `
            <div class="flex items-center justify-center flex-col min-h-screen">
                <div class="mb-[20px]">
                    <img src="./assets/images/settingLogo.png" />
                    <div class="text-2xl text-center mt-[20px]">자동 로그인 설정</div>
                </div>
                <div class="mb-[20px] w-[400px]">
                    <div class="mb-[4px] text-lg">로그인 정보</div>
                    <div class="bg-[#343437] rounded-[8px] px-[20px] py-[12px]">
                        <div class="flex justify-between items-center pb-[12px]">
                            <div class="text-sm">아이디</div>
                            <div class="text-sm text-right">${loginID}</div>
                        </div>
                        <div class="flex justify-between items-center py-[12px]">
                            <div class="text-sm">비밀번호</div>
                            <input
                                type="password"
                                id="msetup_login_pw"
                                class="input-reset text-sm text-right"
                                placeholder="PASSWORD"
                            />
                        </div>
                        <div class="flex justify-between items-center pt-[12px]">
                            <div class="text-sm">자동로그인 사용</div>
                            <div class="el-checkbox">
                                <input type="checkbox" id="msetup_login_auto" name="check" value="option" ${autoChk} />
                                <label for="msetup_login_auto" class="el-checkbox-style"></label>
                            </div>
                        </div>
                    </div>
                </div>
                <div class="flex justify-center items-center mt-[20px]">
                    <div id="msetup_btn_prev" class="bg-[#454548] px-[82px] py-[12px] rounded-[8px] cursor-pointer">이전</div>
                    <div id="reset-btn" class="bg-[#1573d0] px-[82px] py-[12px] rounded-[8px] cursor-pointer ml-[20px]">다음</div>
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
                                    id="msetup_btn_next"
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
        gDOM("main_area").innerHTML = html;
        gDOM("msetup_btn_prev").addEventListener("click", this.onClick_changeStep.bind(this, false));
        gDOM("msetup_btn_next").addEventListener("click", this.onClick_changeStep.bind(this, true));
        gDOM("reset-btn").addEventListener("click", this.onClick_start_countdown.bind(this, true));
        gDOM("reset-cancel-btn").addEventListener("click", this.onClick_start_countdown.bind(this, false));
    }

    _setState_step3() {
        this.curStep = 3;
        let html = `
            <center><b>MAINTX 시스템 설정</b></center><br/><br/>
            <center>시스템 설정이 진행 중입니다.<br/>잠시만 기다려 주세요.....</center><br/><br/>            
        `;

        //console.log(html);
        gDOM("main_area").innerHTML = html;
    }

    _setState_done() {
        this.curStep = 4;
        let html = `
            <center><b>MAINTX 시스템 설정</b></center><br/><br/>
            <center>시스템 설정이 완료되었습니다.<br/>시스템을 재부팅 하세요.</center><br/><br/>
            <center><button id="msetup_reboot">재부팅</button></center>
        `;

        //console.log(html);
        gDOM("main_area").innerHTML = html;
        gDOM("msetup_reboot").addEventListener("click", this.onClick_reboot.bind(this));
    }

    _refreshClusters() {
        let dom = gDOM("mset_vrrp_clusters");

        let html = "";
        for (let i = 0; i < this.sysInfo.vrrp.clusters.length; i++) {
            let curCluster = this.sysInfo.vrrp.clusters[i];
            html += `
                <div class="flex justify-between items-center bg-[#343437] rounded-[8px] m-[8px] px-[10px] py-[12px]">
                    <div class="text-sm">${curCluster.id}</div>
                    <div class="flex justify-center items-center">
                        <div class="text-sm mr-[10px]">${curCluster.ip}</div>
                        <div id="meset_cluster_del_${i}" class="text-center text-sm cursor-pointer">
                            <img src="./assets/images/closeOutline.png" />
                        </div>
                    </div>
                </div>
            `;
        }
        dom.innerHTML = html;

        for (let i = 0; i < this.sysInfo.vrrp.clusters.length; i++) {
            gDOM("meset_cluster_del_" + i).addEventListener("click", this.onClick_cluster_del.bind(this, i));
        }
    }

    _countdown() {
        let countdownTime = 10;
        let countdownElement = document.getElementById("reset-count");

        this.countdownInterval = setInterval(() => {
            countdownElement.innerHTML = countdownTime + " 초 후 재시작됩니다.";
            countdownTime--;

            if (countdownTime < 0) {
                clearInterval(this.countdownInterval);
                countdownElement.innerHTML = "재부팅 시도중...";
                this.onClick_changeStep(true);
            }
        }, 1000);
    }

    onClick_cluster_modal(val) {
        if (val) {
            document.getElementById("modal-cluster").classList.remove("hidden");
        } else {
            document.getElementById("modal-cluster").classList.add("hidden");
        }
    }

    onClick_start_countdown(val) {
        if (val) {
            document.getElementById("modal-reset").classList.remove("hidden");
            this._countdown();
        } else {
            clearInterval(this.countdownInterval);
            document.getElementById("reset-count").innerHTML = "10 초 후 재시작됩니다.";
            document.getElementById("modal-reset").classList.add("hidden");
        }
    }

    onClick_cluster_add() {
        let sId = gDOM("msetup_cluster_id_new").value;
        let sIp = gDOM("msetup_cluster_ip_new").value;
        if (sId.length < 1 || sIp.length < 1) return;

        this.sysInfo.vrrp.clusters.push({ id: sId, ip: sIp });
        this._refreshClusters();
        document.getElementById("modal-cluster").classList.add("hidden");
    }

    onClick_cluster_del(index) {
        if (index < 0 || index >= this.sysInfo.vrrp.clusters.length) return;
        this.sysInfo.vrrp.clusters.splice(index, 1);
        this._refreshClusters();
    }

    onClick_changeStep(bNext) {
        if (bNext) {
            if (this.curStep == 0) {
                let dom = gDOM("msetup_input_dante_id");
                //if (dom.value.length < 1 || dom.value == "MAINTX XX:XX:XX") {
                if (dom.value.length < 1) {
                    alert("Dante device id를 입력하세요");
                    return;
                }
                this.sysInfo.dante_dev_id = dom.value;
                dom = gDOM("msetup_vrrp_use");
                if (!dom.checked) this.sysInfo.use_vrrp = false;
                else {
                    dom = gDOM("msetup_vrrp_sys_id");
                    this.sysInfo.vrrp.id = parseInt(dom.value);
                    if (gDOM("msetup_vrrp_chk_master").checked) this.sysInfo.vrrp.is_master = true;
                    else this.sysInfo.vrrp.is_master = false;
                    dom = gDOM("msetup_vrrp_priority");
                    this.sysInfo.vrrp.priority = parseInt(dom.value);
                    dom = gDOM("msetup_vrrp_netif");
                    if (dom.value.length < 1) {
                        alert("Network interface 를 입력하세요");
                        return;
                    }
                    this.sysInfo.vrrp.interface = dom.value;
                    dom = gDOM("msetup_vrrp_ip");
                    if (dom.value.length < 1) {
                        alert("서비스IP 를 입력하세요");
                        return;
                    }
                    this.sysInfo.vrrp.ip = dom.value;
                    if (this.sysInfo.vrrp.clusters.length < 1) {
                        alert("하나 이상의 클러스터 정보를 입력하세요");
                        return;
                    }
                    for (let i = 0; i < this.sysInfo.vrrp.clusters.length; i++) {
                        let sId = gDOM("msetup_cluster_id_" + i).value;
                        let sIp = gDOM("msetup_cluster_ip_" + i).value;
                        if (sId.length < 1 || sIp.length < 1) {
                            alert("클러스 ID/IP를 입력하세요");
                            return;
                        }
                        this.sysInfo.vrrp.clusters[i].id = sId;
                        this.sysInfo.vrrp.clusters[i].ip = sIp;
                    }
                    this.sysInfo.use_vrrp = true;
                }
                gDOM("msetup_btn_next").style.display = "none";

                let jsv = { act: "SET_SYS_CONFIG", payload: this.sysInfo };
                this.funcCallNative(JSON.stringify(jsv));
            } else if (this.curStep == 1) {
                let tUart = {};
                let dom = gDOM("msetup_emr_type");

                tUart.type = dom.value;

                if (tUart.type === "TCPIP") {
                    dom = gDOM("msetup_emr_tcp_ip");
                    if (dom.value.length < 1) {
                        alert("장비 IP 주소를 입력하세요.");
                        return;
                    }
                    tUart.target_ip = dom.value;

                    dom = gDOM("msetup_emr_tcp_port");
                    if (dom.value.length < 1) {
                        alert("장비 포트번호를 입력하세요.");
                        return;
                    }
                    tUart.target_port = parseInt(dom.value);

                    let checkedValue = null;
                    let radioButtons = document.querySelectorAll('input[type="radio"][name="networkIF"]');

                    radioButtons.forEach((radioButton) => {
                        if (radioButton.checked) checkedValue = radioButton.value;
                    });

                    if (checkedValue === "fif") {
                        let tFixUart = {};

                        dom = gDOM("msetup_emr_net_name");
                        if (dom.value.length < 1) {
                            alert("인터페이스 명칭을 입력하세요.");
                            return;
                        }
                        tFixUart.interface = dom.value;

                        radioButtons = document.querySelectorAll('input[type="radio"][name="addressType"]');
                        radioButtons.forEach((radioButton) => {
                            if (radioButton.checked) checkedValue = radioButton.value;
                        });
                        tFixUart.type = checkedValue;

                        if (checkedValue === "STATIC") {
                            dom = gDOM("msetup_emr_cluster_ip");
                            if (dom.value.length < 1) {
                                alert("고정 IP주소를 입력하세요.");
                                return;
                            }
                            tFixUart.static_ip = dom.value;

                            dom = gDOM("msetup_emr_cluster_mask");
                            if (dom.value.length < 1) {
                                alert("고정 서브넷 마스크를 입력하세요.");
                                return;
                            }
                            tFixUart.static_netmask = dom.value;

                            dom = gDOM("msetup_emr_cluster_gateway");
                            if (dom.value.length < 1) {
                                alert("고정 gateway주소를 입력하세요.");
                                return;
                            }
                            tFixUart.static_gateway = dom.value;

                            dom = gDOM("msetup_emr_cluster_dns1");
                            if (dom.value.length < 1) {
                                alert("고정 DNS1을 입력하세요.");
                                return;
                            }
                            tFixUart.static_dns1 = dom.value;

                            dom = gDOM("msetup_emr_cluster_dns2");
                            if (dom.value.length < 1) {
                                alert("고정 DNS2을 입력하세요.");
                                return;
                            }
                            tFixUart.static_dns2 = dom.value;
                        }

                        tUart.net = tFixUart;
                    }
                } else if (tUart.type === "RS232") {
                    dom = gDOM("msetup_emr_rs_name");
                    if (dom.value.length < 1) {
                        alert("장치명을 입력하세요.");
                        return;
                    }
                    tUart.dev_name = dom.value;

                    dom = gDOM("msetup_emr_rs_rate");
                    if (dom.value.length < 1) {
                        alert("속도를 입력하세요.");
                        return;
                    }
                    tUart.baudrate = parseInt(dom.value);
                }

                this.sysInfo.uart = tUart;
                // console.log(JSON.stringify(this.sysInfo));
                let jsv = { act: "SET_SYS_CONFIG", payload: this.sysInfo };
                this.funcCallNative(JSON.stringify(jsv));
            } else if (this.curStep == 2) {
                let dom = gDOM("msetup_login_auto");
                this.loginInfo.auto = dom.checked;
                //dom = gDOM("msetup_login_id");
                //if (dom.value.length < 1) {
                //    alert("아이디를 입력하세요");
                //    return;
                //}
                //this.loginInfo.id = dom.value;
                dom = gDOM("msetup_login_pw");
                if (dom.value.length < 1) {
                    if (this.loginInfo.auto) {
                        alert("패스워드를 입력하세요");
                        return;
                    }
                    this.loginInfo.pw = "NONE";
                } else this.loginInfo.pw = SHA256(dom.value);
                gDOM("msetup_btn_next").style.display = "none";

                let jsv = { act: "SET_LOGIN_INFO", payload: this.loginInfo };
                this.funcCallNative(JSON.stringify(jsv));
            }
        } else {
            if (this.curStep <= 0) {
                SwitchPage("common_setup");
            } else if (this.curStep == 1) {
                this._setState_step0();
            } else if (this.curStep == 2) {
                this._setState_step1();
            } else {
                this._setState_step2();
            }
        }
    }

    onClick_reboot() {
        // let dom = gDOM('msetup_reboot');
        // dom.style.display = 'none'; //더블클릭 방지

        let jsv = { act: "SYS_REBOOT", payload: "NONE" };
        this.funcCallNative(JSON.stringify(jsv));
    }

    onClick_use_vrrp() {
        let dom = gDOM("msetup_vrrp_use");

        let bVRRP = true;
        if (dom.checked) {
            document.getElementById("msetup-vrrp-mode-1").style.display = "block";
            document.getElementById("msetup-vrrp-mode-2").style.display = "block";
            bVRRP = false;
        } else {
            document.getElementById("msetup-vrrp-mode-1").style.display = "none";
            document.getElementById("msetup-vrrp-mode-2").style.display = "none";
        }
        if (bVRRP) this._refreshClusters();
    }

    onClick_vrrp_change_master(bIsMaster) {
        let domM = gDOM("master-chk");
        let domS = gDOM("slave-chk");
        if (bIsMaster) {
            domM.checked = true;
            domS.checked = false;
        } else {
            domM.checked = false;
            domS.checked = true;
        }
    }

    toggle_connection_type() {
        let selValue = gDOM("msetup_emr_type").value;
        let tcpConn = gDOM("msetup_emr_tcp");
        let rsConn = gDOM("msetup_emr_rs");

        if (selValue === "TCPIP") {
            tcpConn.style.display = "block";
            rsConn.style.display = "none";
        } else if (selValue === "RS232") {
            tcpConn.style.display = "none";
            rsConn.style.display = "block";
        } else {
            tcpConn.style.display = "none";
            rsConn.style.display = "none";
        }
    }

    toggle_network_type() {
        let checkedValue = null;
        let radioButtons = document.querySelectorAll('input[type="radio"][name="networkIF"]');

        radioButtons.forEach((radioButton) => {
            if (radioButton.checked) {
                checkedValue = radioButton.value;
            }
        });

        let nType = gDOM("msetup_emr_net_type");

        if (checkedValue === "mif") {
            nType.style.display = "none";
        } else if (checkedValue === "fif") {
            nType.style.display = "block";
        } else {
            nType.style.display = "none";
        }
    }

    toggle_address_type() {
        let temp = gDOM("msetup_emr_cluster");
        let checkedValue = null;
        let radioButtons = document.querySelectorAll('input[type="radio"][name="addressType"]');

        radioButtons.forEach((radioButton) => {
            if (radioButton.checked) {
                checkedValue = radioButton.value;
            }
        });

        if (checkedValue === "DHCP") {
            temp.style.display = "none";
        } else if (checkedValue === "STATIC") {
            temp.style.display = "block";
        } else {
            temp.style.display = "none";
        }
    }
};

export { maintxSetup };
