# Hitek Linux UI App

Hitek Tinkerboard 3N 보드용 UI Application
Remote - MainTX 공용
Webkit2GTK Base WebUI

## Embeded 빌드환경
- apt update
- apt install build-essential
- apt install -y libgtk-3-dev libwebkit2gtk-4.0-dev

## GIT 최초 설치
- git config --global user.name "Ryu Lord of Darkness"
- git config --global user.email "dark@5universe.com"
- 이후 ID/PW저장
- git config credential.helper store

## 리모트 TLV320AIC34 코덱 구성
- CH1 - AIN1 - A:LINE2L --> LADC
- CH2 - AIN2 - A:LINE2R --> RADC
- CH3 - AIN3 - B:LINE1L --> LADC
- CH4 - AIN4 - B:LINE1R --> RADC
- OUT - DAOUT - A:LEFT_LO <-- LDAC
    ### A Setting
    - REG 7 (0x07): 0x08 - Left-DAC data path plays left-channel input data
    - REG 15 (0x0F): 0x40 - LADC PGA Gain 32dB
    - REG 16 (0x10): 0x40 - RADC PGA Gain 32dB
    - REG 19 (0x13): 0x7C - LINE1L to LADC - Not connected, LADC Power Up
    - REG 20 (0x14): 0xC0 - LINE2L to LADC - differention mode, Input gain -12dB, NC - no common-mode (Check point)
    - REG 22 (0x16): 0x7C - LINE1R to RADC - Not connected, RADC Power Up
    - REG 23 (0x17): 0xC0 - LINE2R to RADC - differention mode, Input gain -12dB, NC - no common-mode (Check point)
    - REG 26 (0x1A): 0x00 - LAGC - disable
    - REG 29 (0x1D): 0x00 - RAGC - disable
    - REG 37 (0x25): 0x80 - LDAC Power Up
    - REG 43 (0x2B): 0x00 - LDAC is not muted
    - REG 82 (0x52): 0x80 - DAC_L1 route to LEFT_LO
    - REG 86 (0x56): 0x9B - LEFT_LO - gain 9dB, LEFT_LO Power Up
    ### B Setting
    - REG 7(0x07): 0x00 - data path is off
    - REG 15 (0x0F): 0x40 - LADC PGA Gain 32dB
    - REG 16 (0x10): 0x40 - RADC PGA Gain 32dB
    - REG 19 (0x13): 0xC4 - LINE1L to LADC - differention mode, Input gain -12dB, LADC Power Up
    - REG 20 (0x14): 0x78 - LINE2L to LADC - Not connected, NC - no common-mode (Check point)
    - REG 22 (0x16): 0xC4 - LINE1R to RADC - differention mode, Input gain -12dB, RADC Power Up
    - REG 23 (0x17): 0x78 - LINE2R to RADC - Not connected, NC - no common-mode (Check point)
    - REG 26 (0x1A): 0x00 - LAGC - disable
    - REG 29 (0x1D): 0x00 - RAGC - disable
    - REG 37 (0x25): 0x00 - default
    - REG 43 (0x2B): 0x80 - LDAC is muted
    - REG 82 (0x52): 0x00 - default
    - REG 86 (0x56): 0x02 - default
