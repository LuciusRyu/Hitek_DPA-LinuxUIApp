const VolumeMain = class volume_main {
    constructor(funcCallNative, connector) {
        this.funcCallNative = funcCallNative;        
        this.connector = connector;
        this.myTXDev = null;
    }

    BuildLayout() {
        if (this.myTXDev == null) this.myTXDev = this.connector.getMyTXDev();
        let szCh = "";
        let i;
        if (this.myTXDev != null) {
            szCh = "<center><table border=1></tbody>";
            szCh += "<tr>";
            for (let txc of this.myTXDev.tx_channels) szCh += "<td>" + txc.nick_name + "</td>";            
            szCh += "</tr>";
            szCh += "<tr>";
            for (i = 0; i < this.myTXDev.tx_channels.length; i++) {
                szCh += `
                    <td>
                        AGC: <div id="vm_attr_cur_${i}">0</div>
                        현재 볼륨<br/>
                        <div id="vm_volume_cur_${i}">0</div>
                    </td>
                `;            
            }
            szCh += "</tr>";
            szCh += "<tr>";
            for (i = 0; i < this.myTXDev.tx_channels.length; i++) {
                szCh += `
                    <td>
                        <input type="number" id="vm_volume_tar_${i}" value=0 /><br/>
                        <button id="vm_volume_set_${i}">변경</button>
                    </td>
                `;            
            }
            szCh += "</tr>";
            szCh += "</tbody></table></center>";
        }

        let html = `
            <table width=100% height=100%><tbody>
                <tr>
                    <td>
                        <center><b>시스템 볼륨</b></center><br/>
                    </td>
                </tr>
                <tr>
                    <td>
                        ${szCh}
                    </td>
                </td>
            </tbody></table>
        `;
        gDOM(DIVID_MAINCTS).innerHTML = html;

        if (this.myTXDev != null) {
            for (i = 0; i < this.myTXDev.tx_channels.length; i++) {
                gDOM("vm_volume_set_" + i).addEventListener('click', this._onVolumeSet.bind(this, i));
            }
        }

        this._getHardwareInfo(false);
        this._getHardwareInfo(true);
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
            }
            else if (jsV.payload.type == "adc_volume") {
                //volumes = [{channel: 0, volume: 0}, ... ]
                let volumes = jsV.payload.volumes;
                if (volumes == null) {
                    console.error("Get Channel volume failed...");
                    return;
                }
                this._setChannelVolume(volumes);
            }
            else {
                console.error("Unkown type...");
                console.log(JSON.stringify(jsV));
            }
        }
        else if (jsV.act == "SET_HARDWARE_INFO_RES") {
            console.log("Set Hardware info done!");
            console.log(JSON.stringify(jsV));
        }
    }

    _getHardwareInfo(bIsVolume) {
        let reqV = { act: "GET_HARDWARE_INFO", payload: "adc_attr" };
        if (bIsVolume) reqV.payload = "adc_volume";
        this.funcCallNative(JSON.stringify(reqV));
    }

    _onVolumeSet(idx) {        
        let vol = [];
        let dom = gDOM("vm_volume_tar_" + idx);
        vol.push({channel: idx, volume: parseInt(dom.value)});

        let reqV = { act: "SET_HARDWARE_INFO", payload: {type: "adc_volume", value: vol}};
        this.funcCallNative(JSON.stringify(reqV));
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
            dom.innerText = " " + attr.attribute;
        }
    }

    _setChannelVolume(volumes) {        
        //volumes = [{channel: 0, volume: 0}, ... ]
        let dom;
        for (let vol of volumes) {
            dom = gDOM("vm_volume_cur_" + vol.channel);
            if (dom == null) {
                console.log("ERROR: Channel for volume is not exist: " + vol.channel);
                continue;
            }
            dom.innerText = " " + vol.volume;
        }
    }
}

export { VolumeMain };
