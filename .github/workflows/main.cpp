#include "utils/logger.h"
#include <coreinit/energysaver.h>
#include <coreinit/thread.h>
#include <wups.h>
#include <wups/config_api.h>
#include <wups/config/WUPSConfigItemBoolean.h>

/**
    Mandatory plugin information.
**/
WUPS_PLUGIN_NAME("Power LED Off");
WUPS_PLUGIN_DESCRIPTION("A WUPS plugin to turn off the Wii U console's front power LED.");
WUPS_PLUGIN_VERSION("v1.0");
WUPS_PLUGIN_AUTHOR("Manus");
WUPS_PLUGIN_LICENSE("GPL");

WUPS_USE_WUT_DEVOPTAB();
WUPS_USE_STORAGE("power_led_off");

#define ENABLE_LED_OFF_CONFIG_ID "enable_led_off"
bool sEnableLedOff = true;

/**
 * SMC I2C Register commands for LEDs
 * Device ID: 0x50 on I2C bus 3
 * 0x11: SMC_CMD_PWRLED_ON
 * 0x12: SMC_CMD_PWRLED_OFF
 * 0x13: SMC_CMD_PWRLED_BLINK
 */

// I2C Write function signature (internal coreinit function)
extern "C" int OSWriteI2C(int bus, int device, const void *data, int size);

void setPowerLedState(bool off) {
    if (off) {
        // SMC_CMD_PWRLED_OFF (0x12)
        uint8_t data[1] = { 0x12 };
        OSWriteI2C(3, 0x50, data, 1);
    } else {
        // SMC_CMD_PWRLED_ON (0x11)
        uint8_t data[1] = { 0x11 };
        OSWriteI2C(3, 0x50, data, 1);
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
    
    WUPSConfigAPIOptionsV1 configOptions = {.name = "Power LED Off"};
    WUPSConfigAPI_Init(configOptions, ConfigMenuOpenedCallback, ConfigMenuClosedCallback);

    WUPSStorageAPI::GetOrStoreDefault(ENABLE_LED_OFF_CONFIG_ID, sEnableLedOff, true);
    WUPSStorageAPI::SaveStorage();

    // Turn off LED if enabled
    if (sEnableLedOff) {
        setPowerLedState(true);
    }
    
    deinitLogging();
}

DEINITIALIZE_PLUGIN() {
    // Restore LED state when plugin is unloaded
    setPowerLedState(false);
}

ON_APPLICATION_START() {
    if (sEnableLedOff) {
        setPowerLedState(true);
    }
}
