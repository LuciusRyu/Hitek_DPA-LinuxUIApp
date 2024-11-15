const MainMenuBar = class main_menu_bar {
    constructor() {
        this.cur_sel = 0;

        this.menuList = [
            { id: "mmb_broadcast", mid: MENUID_BROADCAST, title: "방송" },
            { id: "mmb_volume", mid: MENUID_VOLUME, title: "볼륨" },
            { id: "mmb_monitor", mid: MENUID_MONITOR, title: "모니터" },
            { id: "mmb_setting", mid: MENUID_SETTING, title: "기본 정보" },
        ];
    }

    Initialize() {
        let szMenu = "";
        let i = 0;
        for (i = 0; i < this.menuList.length; i++) {
            szMenu +=
                "<button class='flex items-center justify-center h-full basis-1/4 text-center hover:text-[16px] hover:text-white hover:border-b-4 hover:border-sky-500";
            if (i == 0) szMenu += " text-[16px] text-white border-b-4 border-sky-500";
            else szMenu += " text-[14px] text-[#959595]";
            szMenu += `' id='${this.menuList[i].id}'>${this.menuList[i].title}</button>`;
        }

        let html = `
            <div class="flex items-center flex-none p-4 space-x-4">
                <svg
                    xmlns="http://www.w3.org/2000/svg"
                    width="24"
                    height="24"
                    viewBox="0 0 24 24"
                    fill="none"
                    stroke="currentColor"
                    stroke-width="2"
                    stroke-linecap="round"
                    stroke-linejoin="round"
                    class="w-8 h-8 text-blue-500"
                >
                    <path d="M4 20h16"></path>
                    <path d="m6 16 6-12 6 12"></path>
                    <path d="M8 12h8"></path>
                </svg>
                <h1 class="text-xl font-bold">Hitek-Remote</h1>
            </div>
            <div class="flex items-center flex-auto h-full">
                ${szMenu}
            </div>
            <div class="flex items-center flex-none p-4 space-x-4">
                <span class="text-gray-400">2022. 07. 22(금) 15:24.12</span>
            </div>
        `;

        gDOM(DIVID_MENUBAR).innerHTML = html;
        for (i = 0; i < this.menuList.length; i++) {
            gDOM(this.menuList[i].id).addEventListener("click", this.on_MenuClick.bind(this, i));
        }
        this.cur_sel = 0;
    }

    on_MenuClick(iID) {
        if (iID == this.cur_sel) return;

        let dom = gDOM(this.menuList[this.cur_sel].id);
        dom.classList.remove("text-[16px]", "text-white", "border-b-4", "border-sky-500");
        dom.classList.add("text-[14px]", "text-[#959595]");
        dom = gDOM(this.menuList[iID].id);
        dom.classList.remove("text-[14px]", "text-[#959595]");
        dom.classList.add("text-[16px]", "text-white", "border-b-4", "border-sky-500");
        this.cur_sel = iID;

        SwitchPage(this.menuList[iID].mid);
    }
};

export { MainMenuBar };
