#ifndef __BLEGAMEPAD_H__
#define __BLEGAMEPAD_H__

#include "ble/BLE.h"
#include "ble/GattAttribute.h"

#define BLE_UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION 0x2902
#define BLE_UUID_DESCRIPTOR_REPORT_REFERENCE 0x2908
#define BLE_UUID_DESCRIPTOR_EXTERNAL_REPORT_REFERENCE 0x2907

enum ButtonState
{
    BUTTON_UP,
    BUTTON_DOWN
};

enum GamepadButton
{
    GAMEPAD_BUTTON_UP = 0x1,
    GAMEPAD_BUTTON_DOWN = 0x2,
    GAMEPAD_BUTTON_RIGHT = 0x4,
    GAMEPAD_BUTTON_LEFT = 0x8,
    GAMEPAD_BUTTON_A = 0x10,
    GAMEPAD_BUTTON_B = 0x20,
    GAMEPAD_BUTTON_SELECT = 0x40,
    GAMEPAD_BUTTON_START = 0x80,
};

#define INPUT_REPORT 0x1
#define OUTPUT_REPORT 0x2
#define FEATURE_REPORT 0x3

#define BOOT_PROTOCOL 0x0
#define REPORT_PROTOCOL 0x1

typedef struct
{
    uint8_t ID;
    uint8_t type;
} report_reference_t;

/** 
 * A class to communicate a BLE Gamepad device
 */
class BluetoothGamepadService
{
  public:
    /**
     * Constructor
     * @param dev BLE device
     */
    BluetoothGamepadService(BLEDevice *device);

    /**
     * Toggle the state of one button
     */
    void setButton(GamepadButton button, ButtonState state);

  private:
    BLEDevice &ble;
    bool connected;

    Ticker reportTicker;
    bool reportTickerIsActive;

    uint8_t protocolMode;
    uint8_t controlPointCommand;
    uint8_t inputReportData[1];

    uint8_t buttonsState;

    void onConnection(const Gap::ConnectionCallbackParams_t *params);
    void onDisconnection(const Gap::DisconnectionCallbackParams_t *params);

    GattAttribute::Handle_t inputReportValueHandle;

    void startReportTicker();

    void stopReportTicker();

    void onDataSent(unsigned count);

    void sendCallback();

    void startAdvertise();

    void startService();
};

#endif /* __BLEGAMEPAD_H__ */
