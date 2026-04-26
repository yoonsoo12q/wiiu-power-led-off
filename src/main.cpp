#include "utils/logger.h"
#include <coreinit/ios.h>
#include <wups.h>
#include <wups/config_api.h>
#include <wups/config/WUPSConfigItemBoolean.h>

WUPS_PLUGIN_NAME("Power LED Off");
WUPS_PLUGIN_DESCRIPTION("A WUPS plugin to turn off the Wii U console's front power LED.");
WUPS_PLUGIN_VERSION("v1.0");
WUPS_PLUGIN_AUTHOR("Manus");
WUPS_PLUGIN_LICENSE("GPL");
WUPS_USE_WUT_DEVOPTAB();
WUPS_USE_STORAGE("power_led_off");

#define ENABLE_LED_OFF_CONFIG_ID "enable_led_off"
bool sEnableLedOff = true;

// SMC 커맨드 정의 (WiiUBrew 기준)
#define SMC_CMD_PWRLED_ON   0x11
#define SMC_CMD_PWRLED_OFF  0x12

void setPowerLedState(bool off) {
    // IOS /dev/smc 를 통해 SMC에 접근
    IOSHandle fd = IOS_Open("/dev/smc", IOS_OPEN_READWRITE);
    if (fd < 0) return;

    // ioctl로 LED 커맨드 전송
    // vec[0]: 커맨드 바이트
    uint8_t cmd = off ? SMC_CMD_PWRLED_OFF : SMC_CMD_PWRLED_ON;
    IOS_Ioctl(fd, cmd, nullptr, 0, nullptr, 0);

    IOS_Close(fd);
}

void boolItemChanged(ConfigItemBoolean *item, bool newValue) {
    if (std::string_view(ENABLE_LED_OFF_CONFIG_ID) == item->identifier) {
        sEnableLedOff = newValue;
        WUPSStorageAPI::Store(item->identifier, newValue);
        setPowerLedState(sEnableLedOff);
    }
}

WUPSConfigAPICallbackStatus ConfigMenuOpenedCallback(WUPSConfigCategoryHandle rootHandle) {
    WUPSConfigCategory root = WUPSConfigCategory(rootHandle);
    try {
        root.add(WUPSConfigItemBoolean::Create(ENABLE_LED_OFF_CONFIG_ID, "Turn off Power LED",
                                               true, sEnableLedOff,
                                               boolItemChanged));
    } catch (std::exception &e) {
        return WUPSCONFIG_API_CALLBACK_RESULT_ERROR;
    }
    return WUPSCONFIG_API_CALLBACK_RESULT_SUCCESS;
}

void ConfigMenuClosedCallback() {
    WUPSStorageAPI::SaveStorage();
}

INITIALIZE_PLUGIN() {
    initLogging();

    WUPSConfigAPIOptionsV1 configOptions = {.name = "Power LED Off"};
    WUPSConfigAPI_Init(configOptions, ConfigMenuOpenedCallback, ConfigMenuClosedCallback);
    WUPSStorageAPI::GetOrStoreDefault(ENABLE_LED_OFF_CONFIG_ID, sEnableLedOff, true);
    WUPSStorageAPI::SaveStorage();

    if (sEnableLedOff) {
        setPowerLedState(true);
    }

    deinitLogging();
}

DEINITIALIZE_PLUGIN() {
    setPowerLedState(false);
}

ON_APPLICATION_START() {
    if (sEnableLedOff) {
        setPowerLedState(true);
    }
}
