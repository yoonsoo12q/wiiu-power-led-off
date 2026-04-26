#include "utils/logger.h"
#include <coreinit/dynload.h>
#include <coreinit/energysaver.h>
#include <coreinit/thread.h>
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

// OSWriteI2C 함수 포인터
typedef int (*OSWriteI2C_t)(int bus, int device, const void *data, int size);
static OSWriteI2C_t sOSWriteI2C = nullptr;

bool initI2C() {
    OSDynLoad_Module coreinit;
    if (OSDynLoad_Acquire("coreinit.rpl", &coreinit) != OS_DYNLOAD_OK) {
        return false;
    }
    if (OSDynLoad_FindExport(coreinit, OS_DYNLOAD_EXPORT_FUNC,
                              "OSWriteI2C", (void **)&sOSWriteI2C) != OS_DYNLOAD_OK) {
        sOSWriteI2C = nullptr;
        OSDynLoad_Release(coreinit);
        return false;
    }
    OSDynLoad_Release(coreinit);
    return true;
}

void setPowerLedState(bool off) {
    if (!sOSWriteI2C) return;
    if (off) {
        uint8_t data[1] = { 0x12 }; // SMC_CMD_PWRLED_OFF
        sOSWriteI2C(3, 0x50, data, 1);
    } else {
        uint8_t data[1] = { 0x11 }; // SMC_CMD_PWRLED_ON
        sOSWriteI2C(3, 0x50, data, 1);
    }
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

    initI2C(); // OSWriteI2C 주소 획득

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
