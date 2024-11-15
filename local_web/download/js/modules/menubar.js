const MainMenuBar = class main_menu_bar {
    constructor() {
        this.cur_sel = 0;

        this.menuList = [
            {id: 'mmb_broadcast', mid: MENUID_BROADCAST, title: '방송'},
            {id: 'mmb_volume', mid: MENUID_VOLUME, title: '볼륨'},
            {id: 'mmb_monitor', mid: MENUID_MONITOR, title: '모니터'},
            {id: 'mmb_setting', mid: MENUID_SETTING, title: '기본 정보'},
        ];
    }

    Initialize() {
        let szMenu = '';
        let i = 0;
        for (i = 0; i < this.menuList.length; i++) {
            szMenu += "<td class='";
            if (i == 0) szMenu += "td_sel";
            else szMenu += "td_click";
            szMenu += `' id='${this.menuList[i].id}'>${this.menuList[i].title}</td>`;
            szMenu += "\n";
        }
        
        let html = `
        <table border=1 width=100% height=40px>
            <tr>
                <td>로고 영역</td>
                ${szMenu}
                <td>시간 표시 영역</td>
            </tr>
        </table>
        `;

        gDOM(DIVID_MENUBAR).innerHTML = html;                
        for (i = 0; i < this.menuList.length; i++) {
            gDOM(this.menuList[i].id).addEventListener('click', this.on_MenuClick.bind(this, i));
        }
        this.cur_sel = 0;
    }

    on_MenuClick(iID) {                
        if (iID == this.cur_sel) return;

        let dom = gDOM(this.menuList[this.cur_sel].id);        
        dom.classList.remove('td_sel');
        dom.classList.add('td_click');
        dom = gDOM(this.menuList[iID].id);
        dom.classList.remove('td_click');
        dom.classList.add('td_sel');
        this.cur_sel = iID;

        SwitchPage(this.menuList[iID].mid);
    }
}

export { MainMenuBar };
