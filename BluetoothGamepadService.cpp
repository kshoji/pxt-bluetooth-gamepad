/** 
 * A class to communicate a BLE Gamepad device
 */
#include "MicroBit.h"
#include "pxt.h"
#include "ble/GapAdvertisingData.h"
#include "ble/GattService.h"
#include "ble/GattCharacteristic.h"
#if !CONFIG_ENABLED(MICROBIT_BLE_DFU_SERVICE)
#include "HIDBatteryService.h"
#include "HIDDeviceInformationService.h"
#endif
#include "BluetoothGamepadService.h"
#include "USBHID_Types.h"

namespace
{

static const uint8_t RESPONSE_HID_INFORMATION[] = {0x11, 0x01, 0x00, 0x03};

static const uint16_t uuid16_list[] = {GattService::UUID_HUMAN_INTERFACE_DEVICE_SERVICE,
                                       GattService::UUID_DEVICE_INFORMATION_SERVICE,
                                       GattService::UUID_BATTERY_SERVICE};

/**
 * Characteristic Data(Report Map)
 */
static const uint8_t REPORT_MAP[] = {
    USAGE_PAGE(1), 0x01,                // Generic Desktop
    USAGE(1), 0x05,                     // Game Pad
    COLLECTION(1), 0x01,                // Collection: Application
        REPORT_ID(1), 0x01,             // Report ID 1
        COLLECTION(1), 0x00,            // Collection: Physical
            USAGE_PAGE(1), 0x09,        // Button
            USAGE_MINIMUM(1), 0x01,     // Button 1
            USAGE_MAXIMUM(1), 0x08,     // Button 8
            LOGICAL_MINIMUM(1), 0x00,   // Logical Min: 0
            LOGICAL_MAXIMUM(1), 0x01,   // Logical Max: 1
            REPORT_COUNT(1), 0x08,      // 8 Buttons
            REPORT_SIZE(1), 0x01,       // 1 Bit per button
            INPUT(1), 0x02,             // Data, Variable, Absolute
        END_COLLECTION(0),
    END_COLLECTION(0),
};

static const uint8_t emptyInputReportData[] = {0};

static const uint8_t ENABLE_NOTIFICATION_VALUE[] = {0x01, 0x00};
static const uint8_t INPUT_DESCRIPTOR_REPORT[] = {0x00, 0x01};
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

#if !CONFIG_ENABLED(MICROBIT_BLE_DFU_SERVICE)
    // Battery Service
    HIDBatteryService batteryService(ble, 100);

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

    GattAttribute inputNotifyDescriptor(BLE_UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION, const_cast<uint8_t *>(ENABLE_NOTIFICATION_VALUE), 2, 2, false);
    GattAttribute inputReportDescriptor(BLE_UUID_DESCRIPTOR_REPORT_REFERENCE, const_cast<uint8_t *>(INPUT_DESCRIPTOR_REPORT), 2, 2, false);
    GattAttribute *inputReportDescriptors[] = { &inputReportDescriptor, &inputNotifyDescriptor };
    GattCharacteristic inputReportCharacteristic(GattCharacteristic::UUID_REPORT_CHAR,
                                                 inputReportData, sizeof(inputReportData), sizeof(inputReportData),
                                                 GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ |
                                                     GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY |
                                                     GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE,
                                                 inputReportDescriptors, 2, false);

    GattCharacteristic reportMapCharacteristic(GattCharacteristic::UUID_REPORT_MAP_CHAR,
                                const_cast<uint8_t *>(REPORT_MAP), sizeof(REPORT_MAP), sizeof(REPORT_MAP),
                                GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ);

    GattCharacteristic hidInformationCharacteristic(GattCharacteristic::UUID_HID_INFORMATION_CHAR,
                                                                             const_cast<uint8_t *>(RESPONSE_HID_INFORMATION), sizeof(RESPONSE_HID_INFORMATION), sizeof(RESPONSE_HID_INFORMATION),
                                                                             GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ);

    GattCharacteristic hidControlPointCharacteristic(GattCharacteristic::UUID_HID_CONTROL_POINT_CHAR,
                                                                              &controlPointCommand, 1, 1,
                                                                              GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE);

    protocolModeCharacteristic.requireSecurity(SecurityManager::SECURITY_MODE_ENCRYPTION_NO_MITM);
    inputReportCharacteristic.requireSecurity(SecurityManager::SECURITY_MODE_ENCRYPTION_NO_MITM);
    reportMapCharacteristic.requireSecurity(SecurityManager::SECURITY_MODE_ENCRYPTION_NO_MITM);
    hidInformationCharacteristic.requireSecurity(SecurityManager::SECURITY_MODE_ENCRYPTION_NO_MITM);
    hidControlPointCharacteristic.requireSecurity(SecurityManager::SECURITY_MODE_ENCRYPTION_NO_MITM);

    GattCharacteristic *gamepadCharacteristics[]{
        &hidInformationCharacteristic,
        &reportMapCharacteristic,
        &protocolModeCharacteristic,
        &hidControlPointCharacteristic,
        &inputReportCharacteristic};

    inputReportValueHandle = inputReportCharacteristic.getValueHandle();

    ble.gap().onConnection(this, &BluetoothGamepadService::onConnection);
    ble.gap().onDisconnection(this, &BluetoothGamepadService::onDisconnection);

    GattService gamepadService(GattService::UUID_HUMAN_INTERFACE_DEVICE_SERVICE, gamepadCharacteristics, sizeof(gamepadCharacteristics) / sizeof(GattCharacteristic *));

    ble.gattServer().addService(gamepadService);

    ble.gattServer().onDataSent(this, &BluetoothGamepadService::onDataSent);
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

void BluetoothGamepadService::onDataSent(unsigned count)
{
    startReportTicker();
}

void BluetoothGamepadService::onConnection(const Gap::ConnectionCallbackParams_t *params)
{
    ble.gap().stopAdvertising();
    connected = true;
}

void BluetoothGamepadService::onDisconnection(const Gap::DisconnectionCallbackParams_t *params)
{
    connected = false;
    stopReportTicker();
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
    if (connected == false)
    {
        return;
    }

    startReportTicker();
}

void BluetoothGamepadService::sendCallback()
{
    if (!connected)
    {
        return;
    }

    if (inputReportData[0] == 0 && buttonsState == 0)
    {
        stopReportTicker();
        return;
    }

    inputReportData[0] = buttonsState;

    ble.gattServer().write(inputReportValueHandle, inputReportData, 1);
}
