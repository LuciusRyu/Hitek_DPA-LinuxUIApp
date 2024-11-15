import { ConnectionManager } from "./connector.js";
import { MainMenuBar } from "./menubar.js";
import { BroadcastMain } from "./broadcast.js";
import { VolumeMain } from "./volume.js";
import { MonitorMain } from "./monitor.js";
import { SettingMain } from "./setting.js";

const RemoteMain = class remote_main {
    constructor(funcCallNative) {
        this.funcCallNative = funcCallNative;        

        this.menu_bar = new MainMenuBar();
        this.connector = new ConnectionManager(funcCallNative);
        this.page_broadcast = new BroadcastMain(funcCallNative, this.connector);
        this.page_volume = new VolumeMain(funcCallNative, this.connector);
        this.page_monitor = new MonitorMain(funcCallNative, this.connector);
        this.page_setting = new SettingMain(funcCallNative, this.connector);
        this.current_page = null;        
        this.page_id = '';
    }

    Initialize() {
        let html = `
        <div id='${DIVID_MENUBAR}'></div>
        <div id='${DIVID_MAINCTS}'></div>
        `;

        gDOM("main_area").innerHTML = html;        
        this.menu_bar.Initialize();
        this.connector.Initialize(this.on_ConnectorEvent.bind(this));
        this.switchPage('broadcast');
    }

    on_NativeCall(jsV) {        
        this.connector.on_NativeCall(jsV);

        if (this.current_page != null) this.current_page.on_NativeCall(jsV);        
    }

    on_ConnectorEvent(szEvt, jsV) {        
        if (this.current_page != null) {
            if (typeof(this.current_page.on_ConnectorEvent) != 'undefined') this.current_page.on_ConnectorEvent(szEvt, jsV);
        }
    }

    switchPage(szPage) {
        if (this.page_id == szPage) { //같은 페이지
            return;
        }

        if(this.current_page != null && typeof(this.current_page.onDisabled) != 'undefined') this.current_page.onDisabled();

        if (szPage == MENUID_BROADCAST) {
            this.current_page = this.page_broadcast;
        }
        else if (szPage == MENUID_VOLUME) {
            this.current_page = this.page_volume;
        }
        else if (szPage == MENUID_MONITOR) {
            this.current_page = this.page_monitor;
        }
        else if (szPage == MENUID_SETTING) {
            this.current_page = this.page_setting;
        }
        else {
            console.error("ERROR: switchPage - Unknown page:", szPage);
            return;
        }
        
        this.page_id = szPage;
        this.current_page.BuildLayout();
    }
}

export { RemoteMain };
