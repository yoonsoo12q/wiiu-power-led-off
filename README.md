# Power LED Off — Wii U Aroma Plugin

Wii U가 실행(부팅)될 때 전면 전원 버튼의 **빨간색 LED를 끄는** Aroma(WUPS) 플러그인입니다.

---

## 동작 원리

Wii U의 전면 LED(빨간/파란/노란)는 SMC(System Management Controller) 칩이 제어합니다.  
SMC는 I2C Bus 3, Device ID `0x50`에 위치하며, 아래 레지스터 명령어로 LED를 제어합니다.

| 레지스터 | 명령어 | 설명 |
|---------|--------|------|
| `0x11` | `SMC_CMD_PWRLED_ON` | 전원 LED 켜기 |
| `0x12` | `SMC_CMD_PWRLED_OFF` | 전원 LED 끄기 |
| `0x13` | `SMC_CMD_PWRLED_BLINK` | 전원 LED 깜빡이기 |

이 플러그인은 `INITIALIZE_PLUGIN()` 및 `ON_APPLICATION_START()` 시점에 `OSWriteI2C(3, 0x50, ...)` 를 통해 `SMC_CMD_PWRLED_OFF(0x12)` 명령을 SMC에 전송합니다.

---

## 빌드 방법

### 요구 사항

- [devkitPro](https://devkitpro.org/) (devkitPPC 포함)
- [WUT (Wii U Toolchain)](https://github.com/devkitPro/wut)
- [WUPS (Wii U Plugin System)](https://github.com/wiiu-env/WiiUPluginSystem)

### 빌드

```bash
export DEVKITPRO=/opt/devkitpro
make
```

빌드 성공 시 `PowerLedOff.wps` 파일이 생성됩니다.

### Docker로 빌드

```bash
# Docker 이미지 빌드 (최초 1회)
docker build . -t powerledoff-builder

# 빌드 실행
docker run -it --rm -v ${PWD}:/project powerledoff-builder make

# 디버그 빌드
docker run -it --rm -v ${PWD}:/project powerledoff-builder make DEBUG=1
```

---

## 설치 방법

1. `PowerLedOff.wps` 파일을 SD 카드의 `sd:/wiiu/environments/[ENVIRONMENT]/plugins/` 폴더에 복사합니다.
   - `[ENVIRONMENT]`는 실제 환경 이름으로 교체합니다 (예: `aroma`).
2. Wii U를 재부팅하여 Aroma 환경으로 진입합니다.
3. 부팅 후 전면 전원 버튼의 빨간 LED가 꺼집니다.

### 설정 메뉴

Aroma 플러그인 설정 메뉴(`L + D-Pad Down + Minus`)에서 **"Turn off Power LED"** 옵션을 통해 LED 끄기 기능을 켜고 끌 수 있습니다.

---

## 주의 사항

- 이 플러그인은 `OSWriteI2C` 내부 함수를 사용합니다. 이 함수는 공식 WUT API에 노출되어 있지 않으므로, Wii U 펌웨어 버전에 따라 동작이 다를 수 있습니다.
- 플러그인이 언로드될 때(`DEINITIALIZE_PLUGIN`) 전원 LED는 자동으로 복원됩니다.
- 빌드 환경: devkitPro + WUT + WUPS

---

## 소스 구조

```
wiiu-power-led-off/
├── Makefile
├── README.md
└── src/
    ├── main.cpp        # 플러그인 메인 로직
    └── utils/
        └── logger.h    # 디버그 로깅 유틸리티
```

---

## 라이선스

GPL
