const SettingMain = class setting_main {
    constructor(funcCallNative, connector) {
        this.funcCallNative = funcCallNative;        
        this.connector = connector;
    }

    BuildLayout() {
        let html = `
            <table width=100% height=100%><tr><td>
              <center><b>기본 정보</b></center><br/>
              </td></tr></table>
        `;
        gDOM(DIVID_MAINCTS).innerHTML = html;
    }

    on_NativeCall(jsV) {        
    }
}

export { SettingMain };
