import { commonSetup } from "./common_setup.js";
import { remoteSetup } from "./remote_setup.js";
import { maintxSetup } from "./maintx_setup.js";
import { intCncSetup } from "./int_cnc_setup.js";

const UIMain = class ui_main {
    constructor(funcCallNative) {
        this.funcCallNative = funcCallNative;        

        this.baseInfo = null;
        this.page_common_setup = new commonSetup(funcCallNative);
        this.page_remote_setup = new remoteSetup(funcCallNative);
        this.page_maintx_setup = new maintxSetup(funcCallNative);
        this.page_icnc_setup = new intCncSetup(funcCallNative);
        this.current_page = null;
    }

    on_NativeCall(jsV) {        
        //console.log("on_NativeCall");
        //console.log(JSON.stringify(jsV));
        if (this.current_page == null) {
            if (jsV.act == "SETUP_START") {
                console.log("Start Setup!!!!!!!!!!");
                this.baseInfo = jsV.payload;                
                this.switchPage("common_setup");
            }
            else {
                console.log("Unknown action!!!!!!! ", jsV.act);
            }
        }
        else {
            //console.log("CurPage: " + this.current_page.page_name);
            this.current_page.on_NativeCall(jsV);
        }
    }

    switchPage(szPage) {
        //console.log("Switch page: " + szPage)
        if (szPage == "common_setup") {
            this.current_page = this.page_common_setup;
        }
        else if (szPage == "setup_main_tx") {
            this.current_page = this.page_maintx_setup;
        }
        else if (szPage == "setup_remote") {
            this.current_page = this.page_remote_setup;
        }
        else if (szPage == "setup_int_cnc") {
            this.current_page = this.page_icnc_setup;
        }
        else {
            console.error("ERROR: switchPage - Unknown page:", szPage);
            return;
        }
        this.current_page.SetBaseInfo(this.baseInfo);                
        this.current_page.BuildLayout();
    }
}

export { UIMain };
