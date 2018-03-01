/** 
 * A class to communicate a BLE Gamepad device
 */
#include "MicroBit.h"
#include "pxt.h"
#include "ble/GapAdvertisingData.h"
#include "ble/GattService.h"
#include "ble/GattCharacteristic.h"
#if !CONFIG_ENABLED(MICROBIT_BLE_DEVICE_INFORMATION_SERVICE)
#include "HIDDeviceInformationService.h"
#endif
#include "BluetoothGamepadService.h"
#include "USBHID_Types.h"

namespace
{

static const uint8_t RESPONSE_HID_INFORMATION[] = {0x11, 0x01, 0x00, 0x03};

static const uint16_t uuid16_list[] = {GattService::UUID_HUMAN_INTERFACE_DEVICE_SERVICE,
                                       GattService::UUID_DEVICE_INFORMATION_SERVICE};

/**
 * Characteristic Data(Report Map)
 */
static const uint8_t REPORT_MAP[] = {
    USAGE_PAGE(1), 0x01,                // Generic Desktop
    USAGE(1), 0x05,                     // Game Pad
    COLLECTION(1), 0x01,                // Collection: Application
        COLLECTION(1), 0x00,            // Collection: Physical
            REPORT_ID(1), 0x01,             // Report ID 1

            // axes
            USAGE_PAGE(1), 0x01,            // Generic Desktop
            USAGE(1), 0x30,                 // X
            USAGE(1), 0x31,                 // Y
            LOGICAL_MINIMUM(1), 0xff,       // Logical Min: -1
            LOGICAL_MAXIMUM(1), 0x01,       // Logical Max: 1
            REPORT_SIZE(1), 0x02,           // 1 Bit per axis
            REPORT_COUNT(1), 0x02,          // 2 axes
            INPUT(1), 0x02,                 // Data, Variable, Absolute

            // buttons
            USAGE_PAGE(1), 0x09,            // Button
            USAGE(1), 0x01,                 // Button A
            USAGE(1), 0x02,                 // Button B
            USAGE(1), 0x0b,                 // Select
            USAGE(1), 0x0c,                 // Start
            LOGICAL_MINIMUM(1), 0x00,       // Logical Min: 0
            LOGICAL_MAXIMUM(1), 0x01,       // Logical Max: 1
            REPORT_SIZE(1), 0x01,           // 1 Bit per button
            REPORT_COUNT(1), 0x04,          // 4 Buttons
            INPUT(1), 0x02,                 // Data, Variable, Absolute
        END_COLLECTION(0),
    END_COLLECTION(0),
};

static const uint8_t INPUT_DESCRIPTOR_REPORT[] = {0x01, 0x01};
static const uint8_t REPORT_MAP_EXTERNAL_REPORT[] = {0x2A, 0x19};
}

static bool isInitializedService = false;

/**
 * Constructor
 * @param dev BLE device
 */
BluetoothGamepadService::BluetoothGamepadService(BLEDevice *dev) : ble(*dev)
{
    if (isInitializedService == false)
    {
        startService();
        isInitializedService = true;
    }
    startAdvertise();
}

void BluetoothGamepadService::startService()
{
    memset(inputReportData, 0, sizeof(inputReportData));
    connected = false;
    protocolMode = REPORT_PROTOCOL;
    reportTickerIsActive = false;

    ble.init();
    ble.securityManager().init(true, false, SecurityManager::IO_CAPS_NONE);

#if !CONFIG_ENABLED(MICROBIT_BLE_DEVICE_INFORMATION_SERVICE)
    // Device Information Service
    PnPID_t pnpID;
    pnpID.vendorID_source = 0x2;
    pnpID.vendorID = 0x0D28;
    pnpID.productID = 0x0204;
    pnpID.productVersion = 0x0100;

    HIDDeviceInformationService hidDeviceInformationService(ble, "BBC", "uBit", &pnpID);
#endif

    // Gamepad Service
    GattCharacteristic protocolModeCharacteristic(GattCharacteristic::UUID_PROTOCOL_MODE_CHAR,
                                                                          &protocolMode, 1, 1,
                                                                          GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE);

    GattAttribute inputReportDescriptor(BLE_UUID_DESCRIPTOR_REPORT_REFERENCE, const_cast<uint8_t *>(INPUT_DESCRIPTOR_REPORT), 2, 2, false);
    GattAttribute *inputReportDescriptors[] = { &inputReportDescriptor };
    GattCharacteristic inputReportCharacteristic(GattCharacteristic::UUID_REPORT_CHAR,
                                                 inputReportData, sizeof(inputReportData), sizeof(inputReportData),
                                                 GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ |
                                                     GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY |
                                                     GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE,
                                                 inputReportDescriptors, 1);

    GattAttribute reportMapExternalReportDescriptor(BLE_UUID_DESCRIPTOR_EXTERNAL_REPORT_REFERENCE, const_cast<uint8_t *>(REPORT_MAP_EXTERNAL_REPORT), 2, 2, false);
    GattAttribute *reportMapDescriptors[] = { &reportMapExternalReportDescriptor };
    GattCharacteristic reportMapCharacteristic(GattCharacteristic::UUID_REPORT_MAP_CHAR,
                                                const_cast<uint8_t *>(REPORT_MAP), sizeof(REPORT_MAP), sizeof(REPORT_MAP),
                                                GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ,
                                                reportMapDescriptors, 1);

    GattCharacteristic hidInformationCharacteristic(GattCharacteristic::UUID_HID_INFORMATION_CHAR,
                                                                             const_cast<uint8_t *>(RESPONSE_HID_INFORMATION), sizeof(RESPONSE_HID_INFORMATION), sizeof(RESPONSE_HID_INFORMATION),
                                                                             GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ);

    GattCharacteristic hidControlPointCharacteristic(GattCharacteristic::UUID_HID_CONTROL_POINT_CHAR,
                                                                              &controlPointCommand, 1, 1,
                                                                              GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE);

    GattCharacteristic *gamepadCharacteristics[]{
        &reportMapCharacteristic,
        &protocolModeCharacteristic,
        &hidControlPointCharacteristic,
        &hidInformationCharacteristic,
        &inputReportCharacteristic,
    };

    GattService gamepadService(GattService::UUID_HUMAN_INTERFACE_DEVICE_SERVICE, gamepadCharacteristics, sizeof(gamepadCharacteristics) / sizeof(GattCharacteristic *));

    ble.gattServer().addService(gamepadService);

    protocolModeCharacteristic.requireSecurity(SecurityManager::SECURITY_MODE_ENCRYPTION_NO_MITM);
    inputReportCharacteristic.requireSecurity(SecurityManager::SECURITY_MODE_ENCRYPTION_NO_MITM);
    reportMapCharacteristic.requireSecurity(SecurityManager::SECURITY_MODE_ENCRYPTION_NO_MITM);
    hidInformationCharacteristic.requireSecurity(SecurityManager::SECURITY_MODE_ENCRYPTION_NO_MITM);
    hidControlPointCharacteristic.requireSecurity(SecurityManager::SECURITY_MODE_ENCRYPTION_NO_MITM);

    inputReportValueHandle = inputReportCharacteristic.getValueHandle();

    ble.gap().onConnection(this, &BluetoothGamepadService::onConnection);
    ble.gap().onDisconnection(this, &BluetoothGamepadService::onDisconnection);

    startReportTicker();
}

void BluetoothGamepadService::startAdvertise()
{
    ble.gap().stopAdvertising();
    ble.gap().clearAdvertisingPayload();

    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED |
                                      GapAdvertisingData::LE_GENERAL_DISCOVERABLE);

    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, (uint8_t *)uuid16_list, sizeof(uuid16_list));

    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::GAMEPAD);

    uint16_t minInterval = Gap::MSEC_TO_GAP_DURATION_UNITS(25);
    if (minInterval < 6)
    {
        minInterval = 6;
    }
    uint16_t maxInterval = minInterval * 2;
    Gap::ConnectionParams_t params = {minInterval, maxInterval, 0, 3200}; // timeout in 32 seconds
    ble.gap().setPreferredConnectionParams(&params);

    ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.gap().setAdvertisingInterval(50);
    ble.gap().setAdvertisingPolicyMode(Gap::ADV_POLICY_IGNORE_WHITELIST);
    ble.gap().startAdvertising();
}

void BluetoothGamepadService::startReportTicker()
{
    if (reportTickerIsActive)
    {
        return;
    }
    reportTicker.attach_us(this, &BluetoothGamepadService::sendCallback, 24000);
    reportTickerIsActive = true;
}

void BluetoothGamepadService::stopReportTicker()
{
    if (!reportTickerIsActive)
    {
        return;
    }
    reportTicker.detach();
    reportTickerIsActive = false;
}

void BluetoothGamepadService::onConnection(const Gap::ConnectionCallbackParams_t *params)
{
    ble.gap().stopAdvertising();
    buttonsState = 0;
    connected = true;
}

void BluetoothGamepadService::onDisconnection(const Gap::DisconnectionCallbackParams_t *params)
{
    connected = false;
    startAdvertise();
}

/**
 * Toggle the state of one button
 */
void BluetoothGamepadService::setButton(GamepadButton button, ButtonState state)
{
    if (state == BUTTON_UP)
    {
        buttonsState &= ~(button);
    }
    else
    {
        buttonsState |= button;
    }
}

void BluetoothGamepadService::sendCallback()
{
    if (!connected)
    {
        return;
    }

    // buttons
    inputReportData[0] = buttonsState & 0xf0;

    // axis
    axisX = 0;
    axisY = 0;
    if (buttonsState & GAMEPAD_BUTTON_LEFT)
    {
        axisX--;
    }
    if (buttonsState & GAMEPAD_BUTTON_RIGHT)
    {
        axisX++;
    }
    if (buttonsState & GAMEPAD_BUTTON_UP)
    {
        axisY--;
    }
    if (buttonsState & GAMEPAD_BUTTON_DOWN)
    {
        axisY++;
    }
    switch (axisX)
    {
        case -1: // left
            inputReportData[0] |= 0x03;
            break;
        case 1: // right
            inputReportData[0] |= 0x01;
            break;
    }
    switch (axisY)
    {
        case -1: // up
            inputReportData[0] |= 0x0c;
            break;
        case 1: // down
            inputReportData[0] |= 0x04;
            break;
    }

    ble.gattServer().write(inputReportValueHandle, inputReportData, 1);
}
