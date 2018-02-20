#include "pxt.h"
#include "BluetoothGamepadService.h"
using namespace pxt;
/**
 * A set of functions to send gamepad commands over Bluetooth
 */
namespace bluetooth
{
static BluetoothGamepadService *pGamepadInstance = nullptr;
static BluetoothGamepadService *getGamepad()
{
    if (pGamepadInstance == nullptr)
    {
        pGamepadInstance = new BluetoothGamepadService(uBit.ble);
    }
    return pGamepadInstance;
}

//%
void gamepadButton(GamepadButton button, ButtonState state)
{
    BluetoothGamepadService *pGamepad = getGamepad();
    pGamepad->setButton(button, state);
}
}
