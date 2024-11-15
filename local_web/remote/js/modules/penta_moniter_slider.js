const PentaMoniterSlide = class penta_moniter_slide {
    constructor(szParentID, initialValue = 0, type = 1) {
        this.ID = szParentID;
        this.btnID = szParentID + "_sld_btn";
        this.fillID = szParentID + "_sld_fill";
        this.realSize = 0;
        this.slideSize = 0;
        this.btnSize = 0;
        this.curValue = initialValue;
        this.btnDown = false;
        this.btnPrePos = 0;
        this.btnPreVal = 0;
        this.parentDom = null;
        this.onChangeCB = null;
        this.type = type;

        if (this.curValue < 0) this.curValue = 0;
        if (this.curValue > 1.0) this.curValue = 1.0;
    }

    _onMouseDown(event) {
        if (this.parentDom == null) return;
        let wsRect = this.parentDom.getBoundingClientRect();
        if (!(event.clientX > wsRect.left && event.clientX < wsRect.right && event.clientY > wsRect.top && event.clientY < wsRect.bottom))
            return;

        let tY = event.clientY - wsRect.top;
        if (tY < this.btnSize / 2) tY = 0;
        else tY -= this.btnSize / 2;
        let tV = 1.0 - tY / this.slideSize;
        if (tV < 0) tV = 0;
        if (tV > 1.0) tV = 1.0;

        this.btnPrePos = event.clientY;
        this.btnPreVal = tV;
        this._SetValue(tV);
        this.btnDown = true;
    }

    _onMouseUp(event) {
        if (this.btnDown) {
            if (this.onChangeCB != null) this.onChangeCB(this.ID, this.curValue);
        }
        this.btnDown = false;
    }

    _onMouseMove(event) {
        if (!this.btnDown) return;
        let gap = (event.clientY - this.btnPrePos) / this.slideSize;
        this._SetValue(this.btnPreVal - gap);
    }

    _resetAll() {
        let dom = document.getElementById(this.ID);
        if (dom == null) return;
        this.parentDom = dom;

        let pstyle = getComputedStyle(dom);
        let pH = parseInt(pstyle.height, 10);
        let btnH = 20;
        this.realSize = pH;
        this.slideSize = pH - btnH;
        this.btnSize = btnH;

        let btnP = parseInt(this.slideSize - this.slideSize * this.curValue);
        let btnV = parseInt(this.curValue * 100);

        let fillP, fillH;
        if (this.type == 0) {
            fillP = btnP + parseInt(this.btnSize / 2);
            fillH = this.realSize - fillP;
        } else {
            fillP = 0;
            fillH = btnP + parseInt(this.btnSize / 2);
        }

        let html = `
            <div class="relative w-full h-full">
                <div class="absolute top-0 w-[8px] h-full transform -translate-x-1/2 left-1/2 border-0 rounded-[22px] bg-gradient-to-b from-red-500 via-yellow-400 to-green-500"></div>
                <div id="${this.fillID}" class="absolute top-0 w-[8px] h-full transform -translate-x-1/2 left-1/2 border-0 rounded-[22px] bg-black"></div>       
                <div
                    id="${this.btnID}" 
                    class="absolute inset-x-0 bottom-0 border-0 left-1/2 transform -translate-x-1/2 w-[30px] h-[20px] rounded-[32px] bg-[#7f8ba8] m-0 text-center cursor-pointer text-[12px] leading-[24px]"
                >
                    <div class="flex items-center justify-center w-full h-full text-[12px]">${btnV}</div>
                </div>                   
            </div>
        `;

        dom.innerHTML = html;
        dom.onresize = this._resetAll.bind(this);
        dom.onmousedown = this._onMouseDown.bind(this);
        dom.onmouseup = this._onMouseUp.bind(this);
        dom.onmousemove = this._onMouseMove.bind(this);
    }

    Show() {
        this._resetAll();
    }

    //funcCB = func(szID, fVal)
    SetOnChangeCallback(funcCB) {
        this.onChangeCB = funcCB;
    }

    SetValue(fVal) {
        if (this.btnDown) return;
        this._SetValue(fVal);
    }

    _SetValue(fVal) {
        if (fVal < 0) fVal = 0;
        if (fVal > 1.0) fVal = 1.0;
        if (fVal == this.curValue) return;
        this.curValue = fVal;
        let btnP = this.slideSize - this.slideSize * this.curValue;
        let btnV = String(parseInt(this.curValue * 100));
        let dom = document.getElementById(this.btnID);
        dom.style.top = btnP + "px";
        dom = document.querySelector(`#${this.btnID} > div`);
        dom.innerText = btnV;

        let fillP, fillH;
        if (this.type == 0) {
            fillP = btnP + parseInt(this.btnSize / 2);
            fillH = this.realSize - fillP;
        } else {
            fillP = 0;
            fillH = btnP + parseInt(this.btnSize / 2);
        }

        dom = document.getElementById(this.fillID);
        dom.style.top = fillP + "px";
        dom.style.height = fillH + "px";
    }

    GetValue() {
        return this.curValue;
    }
};

export { PentaMoniterSlide };
